require 'sinatra'

set :bind, '0.0.0.0'
set :port, 4567

LATEST_VERSION = 0 

get '/firmware' do
    partition = params['partition'].to_i

    # MCU wants firmware for the *other* slot
    if partition == 0
        send_file "firmware/firmware_b_v#{LATEST_VERSION}.bin"
    else
        send_file "firmware/firmware_a_v#{LATEST_VERSION}.bin"
    end
end
