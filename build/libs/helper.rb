require 'pathname'

RUBY_REQUIRED_VERSION = '1.8.7'
CMAKE_REQUIRED_VERSION = '3.0.0'

def version_outdated?(current, required)
  (Gem::Version.new(current.dup) < Gem::Version.new(required.dup))
end

def check_versions()

  # Ruby version check
  ruby_err = "LoomSDK requires ruby version #{RUBY_REQUIRED_VERSION} or newer.\nPlease go to https://www.ruby-lang.org/en/downloads/ and install the latest version."
  abort(ruby_err) if version_outdated?(RUBY_VERSION, RUBY_REQUIRED_VERSION)

  # CMake version check
  cmake_ver_groups = /cmake version (\d+\.\d+.\d+)/.match(%x[cmake --version].lines.first)
  
  $CMAKE_VERSION = cmake_ver_groups.length >= 2 ? cmake_ver_groups[1] : nil
  
  cmake_err = "LoomSDK requires CMake version #{CMAKE_REQUIRED_VERSION} or above.\nPlease go to http://www.cmake.org/ and install the latest version."
  abort(cmake_err) if (!installed?('cmake') || !$CMAKE_VERSION || version_outdated?($CMAKE_VERSION, CMAKE_REQUIRED_VERSION))

end

def installed?(tool)
  cmd = "which #{tool}" unless ($HOST.is_a? WindowsHost)
  cmd = "where /Q #{tool}" if ($HOST.is_a? WindowsHost)
  %x(#{cmd})
  return ($? == 0)
end

# return path relative to sdk
def pretty_path(path)
  pathn = Pathname.new path
  sdk = Pathname.new $ROOT
  return pathn.relative_path_from sdk
end

# TODO remove
def writeStub(platform)
  FileUtils.mkdir_p("artifacts")
  File.open("artifacts/README.#{platform.downcase}", "w") {|f| f.write("#{platform} is not supported right now.")}
end

def cp_r_safe(src, dst)
  if File.exists? src
    FileUtils.mkdir_p(dst)
    FileUtils.cp_r(src, dst)
  end
end

def cp_safe(src, dst)
  if File.exists? src
    FileUtils.mkdir_p(File.dirname(dst))
    FileUtils.cp(src, dst)
  end
end

def rm_rf_persistent(path)
  start_time = nil
  time_limit = 60
  Dir.glob(path + '/**/*') { |f|
    next if File.directory?(f)
    while true
      begin
        File.delete(f)
        if start_time
          puts "Removed successfully!"
          start_time = nil
        end
        break
      rescue SystemCallError => e
        break if e.errno == Errno::ENOENT
        start_time = Time.now if !start_time
        time_left = time_limit - (Time.now - start_time)
        throw "Timed out trying to remove #{f}" if time_left < 0
        puts e
        puts "Unable to remove file, retrying for another #{time_left.ceil}s!"
        sleep 1
      end
    end
  }
  FileUtils.rm_rf path
end

def unzip_file (file, destination)
  Zip::File.open(file) do |zip_file|
    zip_file.each do |f|
      f_path=File.join(destination, f.name)
      FileUtils.mkdir_p(File.dirname(f_path))
      zip_file.extract(f, f_path)
    end
  end
end