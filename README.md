# var2
**var2** is a platform which connects programs/devices which talk differenet protocols. As the current state, it supports HTTP and MQTT. The project aims to add support to CoAP and other popular application layer protocols in the future.

# Building
As the current state, the project is only built and tested under Linux systems. But this doesn't mean that you cannot build the project in Windows. Please check the prerequisites before trying.

### Prerequisites
- GCC
- libuv v1.x | For installing libuv, visit [libuv's GitHub repository](https://github.com/libuv/libuv).
- Node.js v4.6.0+
- npm v2.15.9+
  
Download the code from [the GitHub repository](https://github.com/wazahm/var2).

To build the `var2-core`, run: 
```
cd src
make
``` 

Install `node_modules` which **var2** depends on:

```
cd src/translator
npm install
```  

# Usage
To start the **var2**, run the following binary and Node.js files:
- `var2` in the directory `src/`
- `http-server.js` in the directory `src/translator/`
- `mqtt-broker.js` in the directory `src/translator/`

Connect some HTTP and MQTT clients with appropriate server and broker and publish, subscribe, GET and POST data.

### NOTE
Due to some reasons, the **url / topic** which HTTP/MQTT clients deals with should only be a ***number of range 0 to 2147483647***. Alphabets and symbols should not be used in url / topic.

**Acceptable Usage**
- topic - `123456`
- topic - `111111`

- url - `/123456`
- url - `/101010`

**Not Acceptable Usage**
- topic - `123/456`
- topic - `123/*`
- topic - `123/+/456`
- topic - `123/#/456`

- url - `123/456`
- url - `123/456/789`

# Documentation
The project is still in its initial stage. We'll try to document everything ASAP.

# Contribution
We appreciate all forms of contributions. Check out the issues which are still open in [the GitHub repository](https://github.com/wazahm/var2). 








