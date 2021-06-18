# oplb (Oystr Proxy - Load Balancer)

[![badge](https://img.shields.io/badge/license-MIT-blue)](https://github.com/oystr-foss/proxy-load-balancer/blob/main/LICENSE)

oplb is a dynamic proxy load balancer that enables us to handle as many ephemeral clients as possible, while being careless about what discovery backend is being used.

### Requirements

* build-essential
* C++17
* cmake
* libboost-dev
* libcurl4-openssl-dev
* libjsoncpp-dev

### Building from source

Just follow the steps below on a Docker image based on Alpine:
```bash
$ git clone https://github.com/oystr-foss/proxy-load-balancer oplb
$ cd oplb
$ ./build-and-install.sh
```

### Configuration
A `proxy.conf` configuration file is expected to exist under `/etc/oplb/`. 

The default configuration looks like:

```markdown
# general config
host=0.0.0.0
port=8080

# interval in seconds between each query to the discovery backend.
refresh_interval=60

# discovery backend
discovery_url=http://localhost:10000
endpoint=/services
```

### Discovery backend
Basically, we expect an endpoint that serves a JSON payload with at least the following strutcture:
```json
{
  "host": "<IP ADDRESS>",
  "port": 8888
}
```

### Running
In order to run the load balancer, just type oplb:

```bash
$ oplb
Listening on: 0.0.0.0:8080

[06/10/2020 12:39:24] 127.0.0.1:36588 -> 127.0.0.1:8888
[06/10/2020 12:39:28] 127.0.0.1:36596 -> 127.0.0.1:8888
```

### Debugging
By default, all logs are sent to stdin/stderr but in the upcoming updates we are going to store them in `/var/log/oplb.log`.

### TODO

* Create/use an http client;
* Limit forking and reuse childs instead of killing them;
* Add tests.
