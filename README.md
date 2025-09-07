# Botnet-CyS
Python C2 written in Flask with a C++ client file designed for Windows systems

The Python C2 features the ability to handle GET requests and POST requests. GET requests can issue commands for the Windows bots or house a file to download and execute. POST requests collect what the GET request commands return, as well as whatever information is asked of the client from the C2. 
The C2 utilizes threading to return GET requests and actively change the command or file being issued, as well as file path verification to ensure the file can be found.

The C++ bot features silent command execution with absolutely no pop-up windows to maintain stealth. Utilizes libcurl for GET and POST requests to the C2. Runs an infinite loop with proper error handling to ensure the file does not close. 
The C++ bot will run commands and POST the results as well as running 'whoami' in terminal to provide identification.

Both components together demonstrate a functional client-server C2 model, highlighting command execution, file delivery, result collection, and resilient background operation.
