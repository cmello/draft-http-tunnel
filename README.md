# draft-http-tunnel

## Introduction

An experimental HTTP tunnel implementing the [HTTP CONNECT negotiation](https://en.wikipedia.org/wiki/HTTP_tunnel). I'm implementing this as a toy project in my free time to learn new C++ features and techniques, inspired by this [excellent article using Rust](https://medium.com/swlh/writing-a-modern-http-s-tunnel-in-rust-56e70d898700).

I also want to add some features to limit the amount of time my kids spend on the Internet. :-)

## How to build

```bash

git clone --recurse-submodules https://github.com/cmello/draft-http-tunnel.git
cd draft-http-tunnel
mkdir build
cd build
cmake ../
make

```

## How to use

* Run the proxy built:

```bash
./draft_http_tunnel
```

* Set the proxy of your web browser to http://localhost:8080. For Safari users, click the Safari menu, Preferences, Advanced, Proxies: Change settings, check both HTTP and HTTPS and fill the Web Proxy Server fields with ```localhost``` and ```8080```.

