//
//  xpl_shader.c
//  xpl-osx
//
//  Created by Justin Bowes on 2012-09-06.
//  Copyright (c) 2012 Informi Software. All rights reserved.
//

#include "xpl_gl.h"

#include <uthash.h>
#include <utlist.h>
#include <bstrlib.h>
#include <glsw.h>
#include <stdio.h>
#include <limits.h>

#include "xpl.h"
#include "xpl_gl_debug.h"
#include "xpl_memory.h"
#include "xpl_platform.h"
#include "xpl_shader.h"
#include "xpl_preprocessor_hash.h"

#define SHADER_CACHE_SIZE 200

typedef struct shader_table_entry {
	char            name[SHADER_NAME_MAX];
	xpl_shader_t    *shader;
	int             refcount;

	UT_hash_handle  hh;
} shader_table_entry_t;

typedef struct xpl_shader_va {
	char                        name[VATTR_NAME_MAX];
	GLint                       va_id;

	UT_hash_handle              hh;
} shader_va_t;

typedef struct xpl_shader_vao_info {
	int                         vao_id;
	shader_va_t             *va_table;

	UT_hash_handle              hh;
} shader_vao_info_t;

typedef struct xpl_uniform_info {
    uint32_t        id;
	char            name[UNIFORM_NAME_MAX];
	GLint           uniform_location;

	UT_hash_handle  hh;
} xpl_uniform_info_t;

typedef struct xpl_shader_node {
	GLuint                  shader_handle;
	struct xpl_shader_node  *prev;
	struct xpl_shader_node  *next;
} xpl_shader_node_t;

// ---------------- private global shader hash table --------------------
static shader_table_entry_t *s_shader_table = NULL;
static char s_version_suffix[8] = { 0 }; // Glue this onto effect key names, e.g. GL32
static char s_shader_suffix[PATH_MAX] = { 0 };
static char s_shader_prefix[PATH_MAX] = { 0 };
// ----------------------------------------------------------------------

xpl_shader_t *xpl_shader_new(const char *name) {
	xpl_shader_t *shader = xpl_calloc_type(xpl_shader_t);
	strcpy(shader->name, name);
	shader->shader_program_nodes = NULL;
	shader->uniform_table = NULL;
	shader->vao_table = NULL;

	shader_table_entry_t *table_entry = xpl_alloc_type(shader_table_entry_t);
	strcpy(table_entry->name, name);
	table_entry->shader = shader;
	table_entry->refcount = 0;
    
    shader->id = glCreateProgram();

	HASH_ADD_STR(s_shader_table, name, table_entry);
	return shader;
}

static const char *get_file_content_for_effect_key(const char *effect_key, const char **error) {
    char full_key[1024];
	snprintf(full_key, 1024, "%s.%s.%s", effect_key, s_version_suffix, XPL_PLATFORM_STRING);
    
	const char *source = 0;
	xpl_resolve_resource_opts_t resource_opts;
	xpl_resource_resolve_opts(&resource_opts, s_shader_prefix);
	for (size_t i = 0; i < 4; ++i) {
		glswClearError();
		glswSetPath(resource_opts.resource_paths[i], s_shader_suffix);
		source = glswGetShader(full_key);
		*error = glswGetError();
		if (! *error) {
            break;
        }
        source = 0;
	}
    
    return source;
}

static bstring source_for_effect_key_internal(const char *effect_key, const char **error, struct bstrList *included_keys, int count, int level) {
    if (level >= 32) {
        LOG_ERROR("Too many levels of includes reading %s, aborting", effect_key);
        *error = "Incomplete includes";
        return NULL;
    }
    
    int my_source_number = count;
    
    if (included_keys->mlen < included_keys->qty + 1) {
        // Bstrlib snaps this up to powers of two, so I can be stupid here.
        bstrListAlloc(included_keys, included_keys->mlen + 1);
    }
    included_keys->entry[included_keys->qty++] = bfromcstr(effect_key);
    
    const char *source = get_file_content_for_effect_key(effect_key, error);
    if (*error) return NULL;
    
    bstring bsource = bfromcstr(source);
    struct bstrList *lines = bsplit(bsource, '\n');
    bdestroy(bsource);
    source = 0;
    
    int line = 0;
    int version = 0;
    char included_effect_key[128];
    for (size_t i = 0; i < lines->qty; ++i) {

        if (sscanf((const char *)lines->entry[i]->data, "#line %d", &line) > 0) {
            bstring reline = bformat("#line %d %d", line, count);
            bassign(lines->entry[i], reline);
            bdestroy(reline);
        } else {
            line++;
        }
        
        if (level > 0 && sscanf((const char *)lines->entry[i]->data, "#version %d", &version) > 0) {
            bstring reversion = bformat("// #version %d", version);
            bassign(lines->entry[i], reversion);
            bdestroy(reversion);
        }

        if (sscanf((const char *)lines->entry[i]->data, "#include \"%[^\"]", included_effect_key) > 0) {
            
            // Did we already include this key?
            bstring bincluded_key = bfromcstr(included_effect_key);
            int already_included = FALSE;
            for (size_t j = 0; j < included_keys->qty; j++) {
                
                // The effect we asked for has to be the same as the effect included.
                // We can't account for glsw hierarchical matching here, because we may
                // be missing tokens.
                
                if (bstrcmp(bincluded_key, included_keys->entry[j]) == 0) {
                    already_included = TRUE;
                    break;
                }
            }
            bdestroy(bincluded_key);
            
            bstring insert_effect = bformat("// #include \"%s\"\n", included_effect_key);
            if (! already_included) {
                bstring effect_content = source_for_effect_key_internal(included_effect_key, error, included_keys, ++count, level + 1);
                if (*error) {
                    return NULL;
                }
                
                bconcat(insert_effect, effect_content);
                bdestroy(effect_content);
                
                // Restore original line numbering
                bstring reline = bformat("#line %d %d", line, my_source_number);
                bconcat(insert_effect, reline);
                bdestroy(reline);

            }
            bassign(lines->entry[i], insert_effect);
            bdestroy(insert_effect);
        }
    }

    bstring newline = bfromcstr("\n");
    bstring bresult = bjoin(lines, newline);
    bdestroy(newline);
    bstrListDestroy(lines);
    return bresult;
}

static char *alloc_source_for_effect_key(const char *effect_key, struct bstrList *included_keys, const char **error) {
    bstring result = source_for_effect_key_internal(effect_key, error, included_keys, 0, 0);
    
    char *char_result = 0;
    if (! *error) {
        char_result = strdup((const char *)result->data);
    }

    bdestroy(result);
    result = NULL;
    
    return char_result;
}

GLuint xpl_shader_add(xpl_shader_t *shader, const GLenum shader_type, const char *effect_key) {

    const char *error = NULL;
    struct bstrList *included_keys = bstrListCreate();
    char *source = alloc_source_for_effect_key(effect_key, included_keys, &error);
    if (source == 0) {
		LOG_ERROR("Couldn't load shader (type: %d, effect_key: %s): %s", shader_type, effect_key, error);
		xpl_gl_breakpoint_func();
		glswGetError();
		glswClearError();
		return GL_FALSE;
	}
    
    // Parse source for #includes, which are also implemented as effect keys

	GLuint shader_handle = glCreateShader(shader_type);
	glShaderSource(shader_handle, 1, (const char **)&source, 0);
	glCompileShader(shader_handle);
    
	GLint compile_status;
	glGetShaderiv(shader_handle, GL_COMPILE_STATUS, &compile_status);

	if (compile_status == GL_FALSE) {
		GLchar messages[1024];
		glGetShaderInfoLog(shader_handle, sizeof (messages), 0, &messages[0]);
		LOG_ERROR("Couldn't compile shader (type: %d, effect_key: %s): \n%s", shader_type, effect_key, messages);
        LOG_DEBUG("Included keys:");
        for (int i = 0; i < included_keys->qty; ++i) {
            LOG_DEBUG("%d\t: %s", i, included_keys->entry[i]->data);
        }
        xpl_gl_breakpoint_func();
        LOG_DEBUG("%s", source);
        xpl_gl_breakpoint_func();
	}
    
    // Clean up
    free(source);    
    bstrListDestroy(included_keys);
    
    if (compile_status == GL_FALSE) {
        return GL_FALSE;
    }
    

	xpl_shader_node_t *node = xpl_alloc_type(xpl_shader_node_t);
	node->shader_handle = shader_handle;
	node->prev = NULL;
	node->next = NULL;
	DL_APPEND(shader->shader_program_nodes, node);
	LOG_TRACE("Added effect key %s to shader %s", effect_key, shader->name);
    
    GL_DEBUG();
    
    shader->linked = false;

	return shader_handle;
}

#ifndef XPL_GLES
void xpl_shader_bind_frag_data_location(xpl_shader_t *shader, GLuint color_number, const char *name) {
    assert(shader);
    assert(shader->id);
    
    shader->frag_data_locations[color_number] = name;
    glBindFragDataLocation(shader->id, color_number, name);
    
    shader->linked = false;
    
    GL_DEBUG();
}
#endif

GLuint xpl_shader_link(xpl_shader_t *shader) {
	assert(shader);

	if (! shader->shader_program_nodes) {
		LOG_ERROR("Can't link a shader with no program nodes");
		return GL_FALSE;
	}

	GLuint program_handle = shader->id;
    xpl_shader_node_t *shader_node;
    DL_FOREACH(shader->shader_program_nodes, shader_node) {
        glAttachShader(program_handle, shader_node->shader_handle);
        GLenum error;
        while ((error = glGetError()) != GL_NO_ERROR) {
            LOG_ERROR("Error linking shader: 0x%x", error);
            xpl_gl_breakpoint_func();
        }
    }
	
	glLinkProgram(program_handle);
	GLint link_status;
	glGetProgramiv(program_handle, GL_LINK_STATUS, &link_status);
	if (link_status == GL_FALSE) {
		GLchar messages[1024];
		glGetProgramInfoLog(program_handle, sizeof (messages), 0, messages);
		LOG_ERROR("Couldn't link shader: \n%s", messages);
		return GL_FALSE;
	}

    shader->linked = true;
    
	LOG_TRACE("Shader %d linked", shader->id);
	return program_handle;
}

GLint xpl_shader_get_uniform(xpl_shader_t *shader, const char *uniform_name) {
	assert(shader);
	assert(shader->id);
	assert(uniform_name);

	xpl_uniform_info_t *uniform_info = NULL;
    uint32_t uniform_hash = PS_HASH(uniform_name);
	HASH_FIND_INT(shader->uniform_table, &uniform_hash, uniform_info);
	if (! uniform_info) {
		uniform_info = xpl_alloc_type(xpl_uniform_info_t);
        uniform_info->id = uniform_hash;
		uniform_info->name[0] = 0;
		strncpy(uniform_info->name, uniform_name, UNIFORM_NAME_MAX);
		uniform_info->uniform_location = glGetUniformLocation(shader->id, uniform_name);

		if (uniform_info->uniform_location == -1) {
			LOG_WARN("Couldn't get uniform location for %s in shader %d:%s", uniform_name, shader->id, shader->name);
		}

		HASH_ADD_INT(shader->uniform_table, id, uniform_info);
	}
	return uniform_info->uniform_location;
}

XPLINLINE shader_va_t *va_new(GLuint program_handle, const char *va_name) {
	shader_va_t *va = xpl_alloc_type(shader_va_t);
	strcpy(&va->name[0], va_name);
	va->va_id = glGetAttribLocation(program_handle, va_name);
	return va;
}

void va_destroy(shader_va_t **ppva) {
	assert(ppva);
	shader_va_t *va = *ppva;
	assert(va);
	xpl_free(va);
	*ppva = NULL;
}

XPLINLINE shader_vao_info_t *vao_info_new(GLuint vao_id) {
	shader_vao_info_t *vao_info = xpl_alloc_type(shader_vao_info_t);
	vao_info->vao_id = vao_id;
	vao_info->va_table = NULL;
	return vao_info;
}

void vao_info_destroy(shader_vao_info_t **ppinfo) {
	assert(ppinfo);
	shader_vao_info_t *info = *ppinfo;
	assert(info);

	shader_va_t *va, *tmp;

	HASH_ITER(hh, info->va_table, va, tmp) {
		HASH_DEL(info->va_table, va);
		va_destroy(&va);
	}

	xpl_free(info);
	*ppinfo = NULL;
}

GLint xpl_shader_get_va(xpl_shader_t *self, xpl_vao_t *vao, const char *va_name) {
	assert(self && self->id && vao && vao->vao_id && va_name && va_name[0]);

	shader_vao_info_t *vao_info = NULL;
	HASH_FIND_INT(self->vao_table, &vao->vao_id, vao_info);
	if (! vao_info) {
		vao_info = vao_info_new(vao->vao_id);
		HASH_ADD_INT(self->vao_table, vao_id, vao_info);
		LOG_TRACE("Added VA name cache for VAO id %d in shader %s",
				vao->vao_id, self->name);
	}
	shader_va_t *va_info = NULL;
	HASH_FIND_STR(vao_info->va_table, va_name, va_info);
	if (! va_info) {
		va_info = va_new(self->id, va_name);
		if (va_info->va_id >= 0) {
			LOG_TRACE("Added VA cache %s in %s at location %d",
					va_name, self->name, va_info->va_id);
		} else {
			// Maybe the shader just doesn't use it.
			LOG_WARN("Couldn't find VA location for %s in %s", va_name, self->name);
		}
		// Cache the result regardless to avoid repeated lookups.
		HASH_ADD_STR(vao_info->va_table, name, va_info);
	}
	return va_info->va_id;
}

static void shader_destroy(xpl_shader_t *shader) {
	// Release shader object from GL
	if (shader->id) {
		glDeleteProgram(shader->id);
	}

	// Free every program node
	xpl_shader_node_t *node;
	while (shader->shader_program_nodes) {
		node = shader->shader_program_nodes;
		DL_DELETE(shader->shader_program_nodes, shader->shader_program_nodes);
		glDeleteShader(node->shader_handle);
		xpl_free(node);
	}

	// Free the uniform table entries
	xpl_uniform_info_t *el, *tmp;

	HASH_ITER(hh, shader->uniform_table, el, tmp) {
		HASH_DEL(shader->uniform_table, el);
		xpl_free(el);
	}

	// Nuke the program
	xpl_free(shader);
}

void xpl_shader_release(xpl_shader_t **ppshader) {
	xpl_shader_t *shader = (*ppshader);

	shader_table_entry_t *table_entry;
	HASH_FIND_STR(s_shader_table, shader->name, table_entry);
	if (! table_entry) {
		LOG_ERROR("Couldn't find a table entry to fetch shader refcount for shader %s", shader->name);
		return;
	}
	table_entry->refcount--;
    assert(table_entry->refcount >= 0);
	LOG_TRACE("Shader %s remaining refcount is %d", shader->name, table_entry->refcount);

	*ppshader = NULL;
}

xpl_shader_t *xpl_shader_get(const char *name) {
	shader_table_entry_t *table_entry;
	xpl_shader_t *shader;

	HASH_FIND_STR(s_shader_table, name, table_entry);
	if (table_entry == NULL) {
        
        if (HASH_COUNT(s_shader_table) >= SHADER_CACHE_SIZE) {
            // purge unused shaders
            LOG_DEBUG("Purging shaders");
            shader_table_entry_t *purge_el, *purge_tmp;
            HASH_ITER(hh, s_shader_table, purge_el, purge_tmp) {
                if (! purge_el->refcount) {
                    shader_destroy(purge_el->shader);
                    HASH_DEL(s_shader_table, table_entry);
                }
            }
        }
        

		shader = xpl_shader_new(name);
		HASH_FIND_STR(s_shader_table, name, table_entry);
		if (table_entry == NULL) {
			LOG_ERROR("Couldn't store table entry for shader %s", name);
			return NULL;
		}
	} else {
		shader = table_entry->shader;
	}
	table_entry->refcount++;

	return shader;
}

int xpl_shaders_init(const char *path_prefix, const char *path_suffix) {
	if (! glswInit()) {
		LOG_ERROR("glswInit failed");
		return FALSE;
	}

	strcpy(s_shader_prefix, path_prefix);
	strcpy(s_shader_suffix, path_suffix);

	// Get the OpenGL version in use and introduce version tokens
#ifdef XPL_GLES
	sprintf(s_version_suffix, "ES2");
#else
	int major, minor;
	glGetIntegerv(GL_MAJOR_VERSION, &major);
	glGetIntegerv(GL_MINOR_VERSION, &minor);
	sprintf(s_version_suffix, "GL%d%d", major, minor);
#endif

	glswAddDirectiveToken("GL20", "#version 110");
	glswAddDirectiveToken("GL21", "#version 120");

	glswAddDirectiveToken("GL30", "#version 130");
	glswAddDirectiveToken("GL31", "#version 140");
	glswAddDirectiveToken("GL32", "#version 150");
	glswAddDirectiveToken("GL33", "#version 330");

	glswAddDirectiveToken("ES1", "/* TODO provide directives for OpenGL ES 1.0 */");
	glswAddDirectiveToken("ES2", "#version 100");

	return TRUE;
}

void xpl_shaders_add_directive(const char *define) {
	glswAddDirectiveToken(NULL, define);
}

xpl_shader_t *xpl_shader_get_prepared(const char *name, const char *vs_name, const char *fs_name) {
	xpl_shader_t *shader = xpl_shader_get(name);
	if (! shader->linked) {
		xpl_shader_add(shader, GL_VERTEX_SHADER, vs_name);
		xpl_shader_add(shader, GL_FRAGMENT_SHADER, fs_name);
        
		if (! xpl_shader_link(shader)) {
			return NULL;
		}
	}
	return shader;
}


void xpl_shaders_shutdown(void) {
	shader_table_entry_t *element, *tmp;

	HASH_ITER(hh, s_shader_table, element, tmp) {
		shader_destroy(element->shader);
        HASH_DEL(s_shader_table, element);
		xpl_free(element);
	}

	HASH_CLEAR(hh, s_shader_table);
}
