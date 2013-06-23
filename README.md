CAPTOR - a Cyanobacterial Arduino-based PhotobioreacTOR
======

The CAPTOR is photobioreactor (pbr) which allows monitoring growth of a cyanobacterial liquid clulture. It uses an Arduino Uno (R3) microcontroller as an interface between the pbr and the computer. While the basic pbr logic is controlled by the microcontroller, a graphical user interface on the computer handles the setup of the pbr parameters and the data recording.

The basic functions of the CAPTOR are currently the illumination of the liquid culture via a dimmable LED panel, the supply with air via an aquarium pump, the monitoring of the optical density of the culture and its temperature.

![CAPTOR setup](https://github.com/roblehmann/captor/blob/master/img/captor.jpg "CAPTOR setup")

Here you can find the CAPTOR pbr firmware, which currently handles the entire logic of the pbr.

The graphical user interface of the CAPTOR can be found [here](https://github.com/roblehmann/captor_interface).