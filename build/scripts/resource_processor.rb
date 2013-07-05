#! /usr/bin/ruby
require 'fileutils'

class Processor
  
  attr_accessor :base, :transformer
  
  def initialize(base, transformers)
    @files = Dir.glob(base)
    @transformers = transformers || []
  end
  
  def process(target)
    FileUtils.mkdir_p target unless File.exists? target and File.directory? target
		puts @files
    @files.each { |file| 
	  	puts @file
      process_r(file, target)
    }
  end
  
  def process_file(input, output)
    @transformers.each { |t|
      if t.can_process? input
        t.process input, output
        break
      end
    }
  end
  
  def process_r(sources, target)
    sources.each { |source| 
      print "#{source} "
      process_file(source, target)
      puts ""
    }
  end
end

class Transformer 
  @@classes = []
end

class FileTransformer < Transformer
  def can_process?(input)
    return false unless (File.exists? input) && ! (File.directory? input)
    can_process_file?(input)
  end
  def target_filename(input, output, new_extension)
    File.join(output, "#{File.basename(input).chomp(File.extname(input))}#{new_extension}")
  end
end

class WAV2AAC < FileTransformer
  def can_process_file?(input)
    File.extname(input).downcase == '.wav'
  end
  def process(input, output)
    target = target_filename(input, output, ".aac")
    print target
    `afconvert -f "adts" -d "aac " #{input} #{target}`
  end
  @@classes << self
end

# class PNGProcess < Transformer
#   def can_process?(input)
#     File.directory? input
#   end
#   def process(input, output)
#     FileUtils.cp_r(input, output)
#     last_segment = input.split.last
#     `$SRCROOT../build/tools/imageoptim-cli/imageOptim -a -d #{output}`
#     `osascript -e "tell application \"ImageOptim\" to quit"`
#     `osascript -e "tell application \"ImageAlpha\" to quit"`
#     FileUtils.mv(File.join(output, last_segment), output)
#   end
#   @@classes << self
# end

class Copy < FileTransformer
  def can_process_file?(input); true; end
  def process(input, output)
    FileUtils.cp(input, output)
  end
  @@classes << self
end

class Mkdir < Transformer
  def can_process?(input)
    return File.directory? input
  end
  def process(input, output)
    FileUtils.mkdir_p(output)
  end
  @@classes << self
end

class Args
  def self.usage
    puts "#{$0} [-t \"transformer[,transformer ...]\"] input_glob output_dir"
    puts "Transformers available:"
    Transformer.classes.each { |c| puts "\t#{c}" }
  end
  def self.process!
    output, *others = ARGV
    inputs = []
    transformers = []
    next_as_transformer = false
    ok = true
    begin
      others.each { |a|
        next_as_transformer = true and next if a == '-t'
        if (next_as_transformer)
          tclasses = a.split /,\ */
          tclasses.each { |tclass| 
            transformers << const_get(tclass).new
          }
        else
          inputs << a
        end
      }
    rescue
      puts "Arguments: \n#{ARGV.join(' ')}"
      ok = false
    end
    puts "output:         #{output}"
    puts "inputs:         #{inputs.join ' '}"
    puts "transformers:   #{transformers.join ' '}"
		if (ok)
    	p = Processor.new(inputs, transformers)
			p.process(output)
		end
  end
end

Args.usage and exit unless ARGV.length >= 2
Args.process!
