# Remote Control Plugin

This plugin allows to control VPX from another computer, also running VPX, on the same local network. This can prove useful
if one wants to use a computer as input (for example a pincab, with nudge sensors) while needing the game to be rendered by 
a more powerful computer (for example to play in VR).


## Setup

Inside VPinballX.ini on the controller computer, you need to add the following setup (replacing with the IP of the other computer on the local area network):
```
[Plugin.RemoteControl]
Enable = 1
RunMode = 1
Host = <<< ip of the powerful computer >>>
Port = 8888
```

And in the VPinballX.ini on the other computer, you need to add:
```
[Plugin.RemoteControl]
Enable = 1
RunMode = 2
Port = 8888
```


## Using

Once setup, run a table on the controller (for example the default table), and a table on the main computer. Input events on the controller will be replicated to the other computer.