# frozen_string_literal: true

require_relative '../spec_helper'

# FTP rides the same enum surface as HTTP in CSFML 3, and Ftp construction is
# display-free, so both fit here.
RSpec.describe SF::Network::Http do
  it 'exposes the FTP and HTTP enum constants' do
    expect(SF::Network::HttpStatus::OK).to eq(200)
    expect(SF::Network::HttpMethod::GET).to eq(0)
    expect(SF::Network::FtpTransferMode::BINARY).to eq(0)
    expect(SF::Network::FtpStatus::OK).to eq(200)
  end

  it 'configures an HTTP request' do
    expect(SF::Network::Http.new).to be_a(SF::Network::Http)

    request = SF::Network::HttpRequest.new
    request.method = :post
    request.uri = '/index'
    request.set_http_version(1, 1)
    request.body = 'payload'
    request.set_field('Accept', '*/*')

    expect(request).to be_a(SF::Network::HttpRequest)
  end

  it 'constructs an FTP client' do
    expect(SF::Network::Ftp.new).to be_a(SF::Network::Ftp)
  end
end
