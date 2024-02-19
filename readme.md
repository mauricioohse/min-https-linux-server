Small HTTPS server to learn how TLS connection and HTTPS/browser files work. Using openssl and linux sockets functions for learning purposes.

I never ended up implementing the HTTPS per se, but the TLS connection and the certificates do work. So the server creates a TLS connection with the browser, and then answers back with HTTP.

Compile with 
```gcc https-server.c -lssl -lcrypto```

the files theKey.pem and fullChain.pem are needed in the same path as the executable. I used self signed certificate using openssl tool, or you could get a free one from Let's encrypt.

You can test if the server is working by accessing url https://localhost:8080/index.html. remember that the browser must trust the same certificate used on the server!

Code mostly based on Nir Litchman video:
https://www.youtube.com/watch?v=6stTRIOsm2E
