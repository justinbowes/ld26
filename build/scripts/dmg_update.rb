#!/usr/bin/ruby

require 'FileUtils'

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

def main
    config = ENV['CONFIGURATION'].downcase.to_sym
    exit(0) unless config == :release

	product_name = ENV['PRODUCT_NAME']
    src_root = ENV['SOURCE_ROOT']
	target_build_dir = ENV['TARGET_BUILD_DIR']
	executable_wrapper = "#{target_build_dir}/#{product_name}.app"
	src_dmg = "#{src_root}/../build/packaging/osx/template.dmg"
    
    print "Working from %s in %s mode\n" % [src_root, config]

    version_header_filename = "%s/_version.h" % src_root
    contents = read_file(version_header_filename)

    success_str, major, minor, revision, build = get_defines(contents, [
        'LAST_BUILD_SUCCEEDED', 'VERSION_MAJOR', 'VERSION_MINOR', 'VERSION_REVISION', 'VERSION_BUILD'
    ])
	volume_name = "#{product_name} #{major}_#{minor}_#{revision}_#{build}"
	volume_title = "#{product_name} #{major}.#{minor}.#{revision}.#{build}"
	target_dmg_prefix = "#{src_root}/../dist/#{volume_name}"
	target_zip = "#{src_root}/../dist/ultrapew.zip"
	target_dmg = "#{target_dmg_prefix}.dmg"
    
	FileUtils.cp src_dmg, target_dmg

    device=`hdiutil attach -readwrite -noverify -noautoopen '#{target_dmg}' | egrep '^/dev/' | sed 1q | awk '{print $1}'`
	`diskutil rename template '#{volume_title}'`
	puts "Device: #{device}"
	
	volume_path = "/Volumes/#{volume_title}"
	File.directory? volume_path or abort "Expected volume path #{volume_path} not found"
	FileUtils.cp_r "#{executable_wrapper}", "#{volume_path}"
	
	`hdiutil detach #{device}`
	`hdiutil convert -format UDZO '#{target_dmg}' -o '#{target_dmg_prefix}.z.dmg'`
	FileUtils.mv "#{target_dmg_prefix}.z.dmg", "#{target_dmg}"
	FileUtils.rm target_zip, :force => true #swallow errors
	zipline = "zip '#{target_zip}' '#{target_dmg}'"
	`#{zipline}`
	puts "Done #{zipline}, see #{target_zip}"
end
    
main()