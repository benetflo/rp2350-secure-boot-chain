require 'sinatra'

def load_port
    File.readlines('../config.h', chomp: true).each do |line|
        if line.include? "HTTP_SERVER_PORT"
            temp = Integer(line.delete("^0-9"))
            if temp.between?(1024, 49151) and temp.is_a? Numeric
                return temp
            end
        end
    end
    puts "Invalid HTTP port number in 'config.h' file, using default port 4567"
    return 4567
end

def load_version
    File.readlines('../config.h', chomp: true).each do |line|
        if line.include? "FIRMWARE_VERSION"
            return Integer(line.delete("^0-9"))
        end
    end
    
    puts "Failed to read FIRMWARE_VERSION from config file"
    return nil
end

def firmware_dir_empty
    fw_files = Dir.glob("firmware/*.bin")
    
    if fw_files.empty?
        return true
    end
    return false
end

port = load_port()
set :bind, '0.0.0.0'
set :port, port

set :server_version, 1 # default version

get '/firmware' do
    
    if firmware_dir_empty()
        status 404
        return "Firmware directory is empty"
    end

    partition = params['partition'].to_i
    client_version = params['version'].to_i

    version = load_version()
    server_version = version || settings.server_version

    if client_version >= server_version
        status 304
        return
    end

    if partition == 0
        send_file "firmware/firmware_b_v#{server_version}.bin"
    else
        send_file "firmware/firmware_a_v#{server_version}.bin"
    end

end