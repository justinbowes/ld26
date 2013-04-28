#pragma once

#ifdef __cplusplus
extern "C" {
#endif

int glswInit(void);
int glswShutdown(void);
int glswSetPath(const char* pathPrefix, const char* pathSuffix);
const char* glswGetShader(const char* effectKey);
const char* glswGetError(void);
void glswClearError(void);
int glswAddDirectiveToken(const char* token, const char* directive);

#ifdef __cplusplus
}
#endif
