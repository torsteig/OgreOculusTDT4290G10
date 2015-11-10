# OgreOculusTDT4290G10
Oculus Rift DK2 support for OGRE 3D. Uses Oculus SDK 0.7.

## Installation
* Download and install the [Oculus Runtime for SDK 0.7.0.0](https://developer.oculus.com/downloads/pc/0.7.0.0-beta/Oculus_Runtime_for_Windows/).
* Download the [Oculus 0.7.0.0 SDK](https://developer.oculus.com/downloads/pc/0.7.0.0-beta/Oculus_SDK_for_Windows/).
* Download the [OGRE 1.9 SDK](http://www.ogre3d.org/download/sdk).
* Download the [GLEW 1.13.0 SDK](http://sourceforge.net/projects/glew/files/glew/1.13.0/).
* Create the three environment variables *OGRE_HOME*, *OCULUS_HOME* and *GLEW_HOME* with the path to the respective SDKs.
* Clone the repository and build the project with CMake.
* Open the project in your IDE. Build the solutions *ALL_BUILD* and *INSTALL*.
* The runnable application can now be found in *dist/bin/OgreOculus.exe*.
* After the initial installation, it is only necessary to build the *OgreOculus* solution.

## Controls
* Move using <kbd>W</kbd><kbd>A</kbd><kbd>S</kbd><kbd>D</kbd> or arrow keys.
* Rotate body using mouse.
* Look around by physically turning your head when wearing the Oculus Rift.
* Hold <kbd>Shift</kbd> to move at double speed.
* Reset position by pressing <kbd>R</kbd>.
* Exit application by pressing <kbd>Esc</kbd>.