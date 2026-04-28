require 'sinatra'

set :bind, '0.0.0.0'
set :port, 4567

# ÄNDRA DEN HÄR FÖR ATT TESTA VERSIONER
VERSION = 0   # <-- ändra till 1, 2, 3 osv

get '/firmware' do
    partition = params['partition'].to_i

    # Bygg filnamnet baserat på VERSION
    if partition == 0
        fw_path = "firmware/firmware_b_v#{VERSION}.bin"
    else
        fw_path = "firmware/firmware_a_v#{VERSION}.bin"
    end

    fw = File.binread(fw_path)

    # Skicka version först (4 bytes little endian)
    header = [VERSION].pack("V")

    content_type 'application/octet-stream'
    body header + fw
end
