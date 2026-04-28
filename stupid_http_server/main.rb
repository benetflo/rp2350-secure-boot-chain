require 'sinatra'

set :bind, '0.0.0.0'
set :port, 4567

# Change this to test versions
VERSION = 1

get '/firmware' do
    partition = params['partition'].to_i

    if partition == 0
        fw_path = "firmware/firmware_b_v#{VERSION}.bin"
    else
        fw_path = "firmware/firmware_a_v#{VERSION}.bin"
    end

    fw = File.binread(fw_path)

    # Send version first (4 bytes little endian)
    header = [VERSION].pack("V")

    content_type 'application/octet-stream'
    body header + fw
end
