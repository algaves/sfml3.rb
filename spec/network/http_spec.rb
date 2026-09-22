# frozen_string_literal: true

require_relative '../spec_helper'

# FTP rides the same enum surface as HTTP in CSFML 3, and Ftp construction is
# display-free, so both fit here.
RSpec.describe SFML::Http do
  it 'exposes the FTP and HTTP enum constants' do
    expect(SFML::HttpStatus::OK).to eq(200)
    expect(SFML::HttpMethod::GET).to eq(0)
    expect(SFML::FtpTransferMode::BINARY).to eq(0)
    expect(SFML::FtpStatus::OK).to eq(200)
  end

  it 'configures an HTTP request' do
    expect(SFML::Http.new).to be_a(SFML::Http)

    request = SFML::HttpRequest.new
    request.method = :post
    request.uri = '/index'
    request.set_http_version(1, 1)
    request.body = 'payload'
    request.set_field('Accept', '*/*')

    expect(request).to be_a(SFML::HttpRequest)
  end

  it 'constructs an FTP client' do
    expect(SFML::Ftp.new).to be_a(SFML::Ftp)
  end
end
