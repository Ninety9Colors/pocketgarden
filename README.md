# PocketGarden

*A procedurally grown garden living inside your computer.*

PocketGarden began as a weekend experiment to learn how graphics and basic networking work in C++. Somewhere along the way it wandered into procedural generation, mesh building, and L-systems, and it turned into something much more interesting than I planned.

![snowfall screenshot](<docs/snowfall screenshot.png>)
<sub>*Two procedurally generated plants in PocketGarden, surrounded by snowfall*

## The Math Behind It

This project is the foundation of my **Mathematics Internal Assessment (IA)** research paper, which explores how Lindenmeyer systems can model fern meshes using 3D geometry.

**[Read the paper (PDF)](<docs/Math IA_compressed.pdf>)**

## What's Inside

**Procedurally generated plants**
Ferns and lilies are grown from procedural rules and turned into real 3D meshes, generated dynamically rather than loaded from models.

**Live, localized weather**
With your own API key, the garden syncs to real weather for your location and responds to it.

**A 3D lighting system, mostly from scratch**
Built on top of [Raylib](https://www.raylib.com/), with the lighting work done by hand rather than leaning on a prebuilt engine.

**Rain, snow, and fog in the shaders**
Weather effects are driven by math in the shaders instead of particle systems or textures.

## Getting Started

PocketGarden is written in C++ using Raylib and has been tested on Windows. A build script is included in the repository.

1. Clone the repo:
   ```
   git clone https://github.com/[your-username]/pocketgarden.git
   ```
2. Make sure Raylib is available in your environment.
3. Run the included build script.
4. Add your weather API key (see below) and launch the executable.

### Weather API Key

To enable live weather syncing, you'll need an API key from [OpenWeather](https://openweathermap.org/). Place it in a file called **api.env** in the project folder. The file should contain one line:

```
12345!@#$%your_key12345
```

Without a key, you can still modify the weather by picking up the in-game tool with *space* and using *LMB* and *RMB* to cycle through weather conditions.