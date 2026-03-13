require 'sinatra'

port = 4567 # default port

File.readlines('../config.h', chomp: true).each do |line|
    if line.include? "HTTP_SERVER_PORT"
        temp = Integer(line.delete("^0-9"))
        if temp.between?(1024, 49151) and temp.is_a? Numeric
            port = temp
        else
            puts "Invalid HTTP port number in 'config.h' file, using default port 4567"
        end
    end
end


set :bind, '0.0.0.0'
set :port, port

get '/' do
    'Hello World!\n'
end