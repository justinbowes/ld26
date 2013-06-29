#!/usr/bin/ruby

def read_file(filename)
    data = []
    f = File.open(filename, "r")
    f.each_line do |line|
        data << line
    end
    f.close
    return data
end

def write_file(filename, contents)
    f = File.open(filename, 'w')
    f.write(contents)
    f.close
end

def replace_define(data, key)
    data.map! { |el|
        match = /#define\s+(\S+)\s+(\S+)/.match(el)
        if match and match[1] == key and block_given?
            "#define %s %s\n" % [ key, yield(match[2]) ]
        else
            el
        end
    }
end


def get_defines(data, key_array)
    results = []
    data.each { |el|
        match = /#define\s+(\S+)\s+(\S+)/.match(el)
        if match
            index = key_array.index(match[1])
            results[index] = match[2] if index
        end
    }
    results
end

def increment_version(version)
    match = /(\d+)(\D*)/.match(version)
    # keep any nondigit suffix
    "%s%s" % [ match[1].to_i + 1, match[2] ]
end

def process_version_header
    src_root = ENV['SOURCE_ROOT']
    config = ENV['CONFIGURATION'].downcase.to_sym
    print "Working from %s in %s mode\n" % [src_root, config]

    version_header_filename = "%s/_version.h" % src_root
    contents = read_file(version_header_filename)

    success_str, major, minor, revision, build = get_defines(contents, [
        'LAST_BUILD_SUCCEEDED', 'VERSION_MAJOR', 'VERSION_MINOR', 'VERSION_REVISION', 'VERSION_BUILD'
    ])
    success = success_str.downcase[0..0] == 't'
    unless success
        print "Last build failed (%s). Maintaining current version numbers\n" % success_str
        return
    end
    print "Last build succeeded (%s). Bumping current version %s.%s.%s.%s" % [ success, major, minor, revision, build ]

    replace_define(contents, 'VERSION_BUILD', &method(:increment_version))
    replace_define(contents, 'LAST_BUILD_SUCCEEDED') { |old| "false" } # set true if we complete build
    config == :release and replace_define(contents, 'VERSION_REVISION', &method(:increment_version))
    write_file(version_header_filename, contents)
end

process_version_header()