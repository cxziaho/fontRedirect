# fontRedirect by cxziaho
Injects path `ur0:` into `SceLibPvf` (instead of `sa0:data/font/pvf/`) and tells it to open `font.otf`.  
Will be used in my **fontInstaller*, so if you want a full tool, just wait for that :)  
Massive thanks to **Rinnegatamante**, **Xerpi** and **devnoname120**, as well as anyone else who helped me in *#henkaku*.  
  
----
  
## Install
Move fontRedirect.suprx to `ur0:tai/` and add to your `config.txt`:
```
*main
ur0:tai/fontRedirect.suprx
```
Name your font `font.otf` (even if its a .ttf) and move it to `ux0:data/font`  
  
## Building   
Assuming you have the [VitaSDK](http://vitasdk.org) toolchain:  
```  
git clone https://github.com/cxziaho/fontRedirect.git  
cd fontRedirect  
cmake .  
make  
```  
and use fontRedirect.suprx  
  
## Uninstall
Remove fontRedirect.suprx from `config.txt` and `ur0:tai/`, then restart your Vita.