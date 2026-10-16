
# Build a Static File HTTP Server in C

**Project Link:** [View Project](https://nextwork.ai/projects/ed6e4a2d-7530-4183-b263-7ff754088dac)

---

![Image](https://nextwork.ai/lively_gold_heroic_goblin/uploads/ed6e4a2d-7530-4183-b263-7ff754088dac_svpq1562)

## Building an HTTP Server from Scratch in C

### Project goals and motivation

In this project, I'm building an HTTP server from scratch in C. It will enable us to send requested static files in response to web browser request. So I can understand how an HTTP server works and How the underlying abstraction under Node.js works.

## Serving Static Files Over HTTP

### Mapping request paths to files

In this step, I'm setting up a helper function for the HTTP reesponece so that I can detecte MIME types. and map the path to the files,

![Image](https://nextwork.ai/lively_gold_heroic_goblin/uploads/ed6e4a2d-7530-4183-b263-7ff754088dac_svpq1562)

### Using stat() and st_size for file I/O

The stat() call retrieves file metadata, and st_size is critical because it provides the exact byte size needed to set the HTTP Content-Length header and control data transfer loops.

## Secret Mission: Dynamic Directory Listing

![Image](https://nextwork.ai/lively_gold_heroic_goblin/uploads/ed6e4a2d-7530-4183-b263-7ff754088dac_slf6xl7w)

### Building HTML responses incrementally with snprintf

The offset variable tracks how many records to skip in a data source, allowing you to fetch and render a specific chunk of data into HTML with each incremental request.

## Setting Up the Project and First Compile

### Project structure and toolchain

In this step, I'm setting up C compiler GCC and runnig a simple C program to make sure the copiler is running properly. Additionaly, I am making a static file that my server will serve.

### Confirming the build and public/ directory

I confirmed that running ./server prints `Server starting...`. Files in public/ directory are index.html and style.css.

## Creating a TCP Socket and Accepting Connections

### socket, bind, listen, and accept

In this step, I'm setting up TCP socket using POSIX socket API to bind it to port 8080. So we can accecpt incoming connection from web browser. 

### Observing live browser connections in the terminal

When I visit localhost:8080, my terminal prints New Connection Received twice.

## Parsing Raw HTTP Requests

### Reading bytes off the wire

In this step, I'm reading raw HTTP request byte into memory buffer. Then parse the HTTP method from thr request line and send 405 error resonse for any other request that GET.

![Image](https://nextwork.ai/lively_gold_heroic_goblin/uploads/ed6e4a2d-7530-4183-b263-7ff754088dac_d8uydgvj)

### How recv() and null-termination work

recv() reads the request send by the brower throgh socket. I null-terminate because I treated incoming data as string, as it was stored in buffer reaseleing it with the null-terminte was nessary to not get overflow.

## Reflections and Key Takeaways

### Tools and concepts mastered

I did this project today I revisited wsl (Ubuntu). build a Server that respond to browser GET Request in pure c.
learn how to create a TCP socket programming and how to bind them to a port.

### Time and challenges

This project took me approximately 5 hourse t of focus to know and learn the the protocal and server, side-by-side.

### What's next

I did this project today to learn how web serves a actual funcions under the hood, benith all the abstraction with high level frameworks.

---