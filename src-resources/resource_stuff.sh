../build/scripts/resource_processor.rb $(SRCROOT)/../dist/audio/ $(SRCROOT)/src-resources/*/*.wav -t WAVCompress
../build/scripts/resource_processor.rb $(SRCROOT)/../dist/fonts/ $(SRCROOT)/src-resources/*/*.ttf */*.otf -t Copy
../build/scripts/resource_processor.rb $(SRCROOT)/../dist/shaders/ $(SRCROOT)/src-resources/*/*.glsl -t Copy

