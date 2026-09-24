# frozen_string_literal: true

# File transfer sits above raw sockets: Http fetches a resource from a web
# server, and Ftp lists a remote directory and moves files to and from disk.
# Both return a status object so you can check ok?/status before trusting the
# payload. This script fetches one URL with Http and, if FTP_HOST is set, logs
# in anonymously and downloads a file. It degrades cleanly offline.
#
# Run from the repository root:
#   bundle exec ruby -Ilib examples/network/file_transfer/file_transfer.rb
#
# Optional environment:
#   HTTP_HOST  host to GET       (default www.sfml-dev.org)
#   HTTP_PATH  path to GET       (default /)
#   FTP_HOST   host to FTP into  (no default; FTP is skipped when unset)

require 'sfml'
require 'fileutils'
require 'tmpdir'
include SF::Network
include SF::System

OUT = File.join(Dir.tmpdir, 'sfml-file-transfer')
FileUtils.mkdir_p(OUT)

def rule(title)
  puts "\n== #{title}"
end

rule 'HTTP GET'
http_host = ENV.fetch('HTTP_HOST', 'www.sfml-dev.org')
http_path = ENV.fetch('HTTP_PATH', '/')
begin
  http = Http.new
  http.set_host(http_host, 80)

  request = HttpRequest.new
  request.method = :get
  request.uri = http_path
  request.set_field('User-Agent', 'sfml3-rb example')

  response = http.send_request(request, Time.seconds(5.0))
  puts "GET http://#{http_host}#{http_path} -> #{response.status} #{response.status_name}"
  puts "content-type: #{response.field('Content-Type').inspect}"
  body = response.body
  puts "received #{body.bytesize} bytes"

  path = File.join(OUT, 'index.html')
  File.binwrite(path, body)
  puts "saved to #{path}"
rescue StandardError => e
  puts "HTTP request failed (offline?): #{e.class}: #{e.message}"
end

rule 'FTP'
ftp_host = ENV.fetch('FTP_HOST', nil)
if ftp_host.nil? || ftp_host.empty?
  puts 'FTP_HOST is not set, so the FTP transfer is skipped.'
  puts 'Set it (and FTP_USER/FTP_PASSWORD) to try a real server:'
  puts '  FTP_HOST=ftp.example.org FTP_USER=user FTP_PASSWORD=secret \\'
  puts '    bundle exec ruby -Ilib examples/network/file_transfer/file_transfer.rb'
else
  begin
    ftp = Ftp.new
    response = ftp.connect(IpAddress.from_string(ftp_host), 21, Time.seconds(5.0))
    puts "connect -> #{response.status} #{response.status_name}"

    user = ENV.fetch('FTP_USER', nil)
    login = user ? ftp.login(user, ENV.fetch('FTP_PASSWORD')) : ftp.login_anonymous
    puts "login -> #{login.status} #{login.status_name}"

    listing = ftp.directory_listing
    puts "directory listing: #{listing.count} entries"
    [listing.count, 10].min.times { |i| puts "  #{listing.name(i)}" }

    # Download the first regular file we can see, then clean up.
    remote = (0...listing.count).map { |i| listing.name(i) }.find { |n| n.include?('.') }
    if remote
      local = File.join(OUT, File.basename(remote))
      download = ftp.download(remote, local, :binary)
      puts "download #{remote.inspect} -> #{download.status} #{download.status_name}"
      puts "  #{File.size(local)} bytes at #{local}" if download.ok?
    else
      puts 'no downloadable file found in the listing'
    end

    ftp.disconnect
  rescue StandardError => e
    puts "FTP failed: #{e.class}: #{e.message}"
  end
end

rule 'Uploading'
puts 'Ftp#upload(local_file, remote_path, mode, append) mirrors #download.'
puts 'SFTP is the modern replacement but is not part of the bound CSFML surface.'
