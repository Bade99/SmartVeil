# Smart Veil
The main goal of this application is to be able to choose a limit below which pixels will not be opaqued; every other program that I know of will darken every pixel on the screen, the problem with this is that you lose a lot of information about the darker parts of the image when really you only want to lower the brighter sections.

By working with each pixel individually we have a lot more control, at the expense of a little more processing power.

This early prototype is the culmination of two months of off and on research and coding, no optimization has been done yet and there's still a few problems, but the idea and its basis have been clearly set and this works as a very good proof of concept, I hope.

Everything has been done on C++ with the Windows API, Desktop Duplication API and DirectX, any pc with windows 8 or above should be able to run this program (the limitation of OS is due to desktop duplication api).

### Main problems:
* **A system is needed to stop updates when no changes are happening on screen.** Right now once the veil is turned on new frames are produced all the time: we receive a new frame from Windows, we generate a veil for it and update the window, which in turn means there is a new frame that Windows will produce, and the cicle starts again. One simple but probably inefficient way to solve this would be to check the newly generated veil against the old one, if they are the same then we wont update the window and the cicle is stopped, until a real new frame is generated by Windows.

* **Flickering.** With almost any configuration of slider values it is noticeable that there is some amount of pixel flickering due to not having enough information to reconstruct the pixel behind the veil. I'm not sure this problem can be completely solved but it can surely be diminished. You can look at PSVeil.hlsl for the current way we are "handling" this.

* **Mouse movemement detected as frame update.** Just moving the mouse is a specific type of frame update where pixels are not changed, only mouse position, it'd be great if we could detect this case of frame updates and completely ignore it since we dont really ever care for where the mouse is, and it's also not affected by the veil. Note: it's possible we already handle this but have forgot to add some way of ignoring a frame update.

* **Windows overhead.** It seems Windows is not too good at managing the type of window needed to make this program work (layered windows) which causes a pretty significant system slowdown and lack of response. I don't have many ideas in this regard so it remains as an open problem.

Feel free to try it, change it, etc. and if you're interested in helping please let me know!

Franco Badenas Abal

Contact info
[twitter](https://twitter.com/francobadenas)
