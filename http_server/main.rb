require 'sinatra'

port = 4567 # default port
server_version = 1 # default version

File.readlines('../config.h', chomp: true).each do |line|
    if line.include? "HTTP_SERVER_PORT"
        temp = Integer(line.delete("^0-9"))
        if temp.between?(1024, 49151) and temp.is_a? Numeric
            port = temp
        else
            puts "Invalid HTTP port number in 'config.h' file, using default port 4567"
        end
    elsif line.include? "FIRMWARE_VERSION"
        server_version = Integer(line.delete("^0-9"))
    end
end


set :bind, '0.0.0.0'
set :port, port

get '/firmware' do

    fw_files = Dir.glob("firmware/*.bin")
    if fw_files.empty?
        puts "Firmware directory is empty!"
    end

    partition = params['partition'].to_i
    client_version = params['version'].to_i

    if client_version >= server_version
        status 304
        return
    end

    if partition == 0
        send_file 'firmware/firmware_b_signed.bin'
    else
        send_file 'firmware/firmware_a_signed.bin'
    end

end