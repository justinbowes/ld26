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


def replace_plist_value(data, key, new_value)
    next_match = false
    data.map! { |el|
        if next_match
            regex_str = "(.*<string>)[^<]+(</string>.*)"
            match = /#{regex_str}/.match(el)
            open, close = match.captures
            next_match = false
            new_value = "%s%s%s\n" % [ open, new_value, close ]
            print "Using #{new_value}"
            new_value
        else
            regex_str = "\s*<key>#{key}</key>\n"
            match = /#{regex_str}/.match(el)
            next_match = !! match
            print "Replacing #{key}\n" if next_match
            el
        end
    }
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

def increment_version(version)
    match = /(\d+)(\D*)/.match(version)
    # keep any nondigit suffix
    "%s%s" % [ match[1].to_i + 1, match[2] ]
end

def process_version_header
    src_root = ENV['SOURCE_ROOT']
    config = ENV['CONFIGURATION'].downcase.to_sym
	info_plist_filename = "#{ENV['TARGET_BUILD_DIR']}/#{ENV['INFOPLIST_PATH']}"
	conInfo="Contents/Info.plist"
	dsym_plist_filename="#{ENV['DWARF_DSYM_FOLDER_PATH']}/#{ENV['DWARF_DSYM_FILE_NAME']}/#{conInfo}"

    print "Working from %s in %s mode\n" % [src_root, config]

    version_header_filename = "%s/_version.h" % src_root
    version_contents = read_file(version_header_filename)

    major, minor, revision, build = get_defines(version_contents, [
        'VERSION_MAJOR', 'VERSION_MINOR', 'VERSION_REVISION', 'VERSION_BUILD'
    ])    
    cf_bundle_version = build.to_s
    cf_bundle_short_version_string = [ major, minor, revision, build ].join '.'

    info_plist_contents = read_file(info_plist_filename)
    replace_plist_value(info_plist_contents, "CFBundleVersion", cf_bundle_version)
    replace_plist_value(info_plist_contents, "CFBundleShortVersionString", cf_bundle_short_version_string)
    write_file(info_plist_filename, info_plist_contents);

    replace_define(version_contents, 'LAST_BUILD_SUCCEEDED') { |old| "true" }
    write_file(version_header_filename, version_contents)
end

process_version_header()