---
layout: default
title: File Transfer
parent: Networking
grand_parent: "Part IV — Advanced"
nav_order: 5
---

# Recipe: File Transfer

Sending a file is a socket protocol with a header — a name, a size, then the bytes. SFML packages two of the common cases for you: `Http` fetches a resource from a web server into memory, and `Ftp` logs into a server to list a directory and move files to and from disk. Both return a status object to check before using the result.

> **Advanced**

## Goal

A console script (no window) that GETs a URL with `Http` and saves the body, then, if `FTP_HOST` is set, logs in, lists a directory and downloads a file. It degrades cleanly when offline or unconfigured.

## How the code works

### 1. Prepare the output directory

The script needs somewhere durable to write the downloads, so `Dir.tmpdir` is joined with `sfml-file-transfer` and `FileUtils.mkdir_p` creates it. A small `rule` helper prints a banner before each section of output.

{% example ruby examples/network/file_transfer/file_transfer.rb 23-28 %}

### 2. HTTP: aim the request

The host and path come from `HTTP_HOST`/`HTTP_PATH` with defaults, and an `Http` object is pointed at the host on port 80 with `set_host`. An `HttpRequest` carries `method = :get`, the `uri`, and a `User-Agent` header added with `set_field`.

{% example ruby examples/network/file_transfer/file_transfer.rb 30-40 %}

### 3. HTTP: send, inspect and save

`send_request(request, Time.seconds(5.0))` returns an `HttpResponse` once the server replies or the timeout fires. The script prints `status`/`status_name` and the `Content-Type` field, then reads `body`, reports its `bytesize`, and `File.binwrite`s it under the output directory; a rescue reports an offline failure instead of aborting.

{% example ruby examples/network/file_transfer/file_transfer.rb 42-53 %}

### 4. FTP: decide whether to run

FTP has no default host, so `FTP_HOST` is read and checked for nil or empty. When unset, the script prints the environment variables to set and moves on — that is what lets the recipe degrade cleanly with no configuration.

{% example ruby examples/network/file_transfer/file_transfer.rb 55-61 %}

### 5. FTP: connect and log in

With a host present, an `Ftp` is created and `connect(IpAddress.from_string(host), 21, timeout)` reports its status. The login is anonymous unless `FTP_USER` is set, in which case `login(user, FTP_PASSWORD)` is used; both print `status`/`status_name`.

{% example ruby examples/network/file_transfer/file_transfer.rb 63-70 %}

### 6. FTP: list the directory

`directory_listing` returns an `FtpListingResponse` whose `count` and `name(index)` walk the entries. The script prints the count and the first ten names, which is also how it finds a file to download.

{% example ruby examples/network/file_transfer/file_transfer.rb 72-74 %}

### 7. FTP: download one file

It scans the listing for the first name containing a dot, treats that as the remote file, and derives a local path from its basename. `download(remote, local, :binary)` writes it to disk, and `File.size` confirms the bytes only when `download.ok?`.

{% example ruby examples/network/file_transfer/file_transfer.rb 76-85 %}

### 8. FTP: disconnect and clean up

`ftp.disconnect` ends the session, and the outer rescue prints any FTP failure rather than raising. The final lines note that `upload(local, remote, mode, append)` mirrors `download`, and that SFTP is the modern replacement but is not part of the bound CSFML surface.

{% example ruby examples/network/file_transfer/file_transfer.rb 87-94 %}

## Ingredients

| Class | Subsystem | Role | Key methods |
| --- | --- | --- | --- |
| `Http` | Network | Web request | `set_host`, `send_request` |
| `HttpRequest` / `HttpResponse` | Network | Request and reply | `method=`, `uri=`, `set_field`; `status`, `status_name`, `field`, `body` |
| `Ftp` | Network | FTP session | `connect`, `login_anonymous`, `login`, `directory_listing`, `download`, `upload`, `disconnect` |
| `FtpResponse` / `FtpListingResponse` | Network | FTP results | `ok?`, `status_name`; `count`, `name` |
| `IpAddress` | Network | FTP host | `from_string` |

See the [Network API]({% link api/network.md %}) for the full signature of each.

## The complete script

The complete program, ready to copy into `examples/network/file_transfer/file_transfer.rb` and run.

{% example ruby examples/network/file_transfer/file_transfer.rb %}

{: .note }
> `Http` and `Ftp` are the older, higher-level protocols still bound in this gem; `Sftp` is SFML 3.1's FTP replacement and is not part of the bound CSFML surface. For a transfer you control, send a header packet followed by the bytes over a [`TcpSocket`]({% link book/net-tcp.md %}).
