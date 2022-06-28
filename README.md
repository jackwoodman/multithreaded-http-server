# Multithreaded HTTP Server
A Multithreaded HTTP Server (HTTP 1.0) that can respond to GET requests.
Responds to IPv4 and IPv6 requests, with valid MIME types.
(The webroot used for testing can be found under www/)

This project was originally built for a major University project, for which it received 98%.
The server was required to be able to handle requests on multiple threads, and to fail gracefully under any possible malformed/illegal input or request.
