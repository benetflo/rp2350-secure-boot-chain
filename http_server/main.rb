require 'sinatra'
require 'json'
require 'openssl'

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

def verify_firmware(bin_path, pubkey_path)
    fw = File.binread(bin_path)
    fw_data = fw[0..-65]
    sig = fw[-64..]
    
    fw_hash = OpenSSL::Digest::SHA256.digest(fw_data)
    
    pubkey = OpenSSL::PKey.read(File.read(pubkey_path))
    pubkey.verify(nil, sig, fw_hash)
rescue => e
    puts "Verification error for #{bin_path}: #{e}"
    false
end

def firmware_dir_empty
    Dir.glob("firmware/*.bin").empty?
end

def load_versions
    JSON.parse(File.read('versions.json'))
end

port = load_port()
set :bind, '0.0.0.0'
set :port, port

# remove files in firmware dir that does not match firmware signature
Dir.glob("firmware/*.bin").each do |f|
    unless verify_firmware(f, '../firmware_public_key.pem')
        puts "WARNING: #{f} failed signature verification, removing"
        File.delete(f)
    end
end

get '/firmware' do
    if firmware_dir_empty()
        status 404
        return "Firmware directory is empty"
    end

    if params['partition'].nil? or params['version'].nil?
        status 400
        return "Firmware argument error: One or more arguments was not defined"
    end

    partition = params['partition'].to_i
    if partition != 0 and partition != 1
        status 400
        return "Firmware partition argument error: Not 0(A) or 1(B)"
    end

    client_version = params['version'].to_i
    if client_version < 0
        status 422
        return "Firmware version argument error: Version can not be a negative number"
    end

    versions = load_versions()
    server_version = versions['current_version']
    min_version = versions['min_version']

    if client_version < min_version
        status 403
        return "Version too old, rollback not permitted"
    end

    if client_version >= server_version
        status 304
        return "Firmware is latest version"
    end

    if partition == 0
        send_file "firmware/firmware_b_v#{server_version}.bin"
    else
        send_file "firmware/firmware_a_v#{server_version}.bin"
    end
end