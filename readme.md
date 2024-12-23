
# Modem

  

Modem is a LAN chat for Windows written in C. It allows for sending text & files over a "dumb" network.
That means it doesn't need dedicated server, only a broker which broadcasts the message to every client.
A simple ncat is enough.

  
## Building

  

Just clone the repo and run ```make``` in the root dir
For building broadcaster just ```cd server``` then ```make```

 
## Running broadcaster

Running the ```broad.exe``` and ```ncat -l port --broker``` simultaneously should be enough but obviously there're some options you can set.

Just look at ```broad.exe /h``` for the list of them.

  

## Running client

Run ```Modem.exe``` and it should just discover the server. If not there's ```/f ip<:port>``` for direct connection.
This program also has help under ```/h```

  

When first ran, Modem will prompt for username which can be at most 23 chars long.
Messages can be 80 chars long.

## File transfer

You can send file using the ```/send``` command. Anyone on the receiving side will automatically download this file to the current directory.

If you want to send a file to specific group of people, use threads.

  

## Private messaging

  

Using the ```/priv+User``` command sends the message to specified user ONLY

  

## Threads

  

Modem has a feature called 'threads' that are, in a way, channels. You can access them via ```/threads+num``` command.

Any message sent on one thread WILL NOT appear on another one, unless user does a SHEBANG.

Multiple users can join one thread

  

## SHEBANGS

  

SHEBANGS are 2-chars long commands, mostly consisting of special chars. These may be used for formatting the message or giving it special attributes.

Three special SHEBANGS must appear at the start of the prompt, these are:

  

!! Used for pinging all users on the thread
\## Used for broadcasting the message on all threads
#! Used for both

  

## Formatting
Formatting uses normal SHEBANGS, these can appear anywhere in the prompt and always start with dollar sign ```$```


$r $g $b - These change the foreground color to R,G,B respectively. They can also mix together e.g $r$g gives yellow

$R $G $B - The same but for background

$> $< - These set foreground and background intensity respectively

$! - Clears formatting

$$ - Just a dollar sign

  

# Happy LAN Party chatting!
