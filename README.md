
## 3D Transformations with OpenGL and ImGui

### Overview
This project involves creating a 3D viewer using OpenGL with both CPU and GPU-based transformations. The viewer supports various transformations such as translation, rotation, scaling, reflection, and shearing. An interactive GUI using ImGui is implemented to control these transformations.

### Project Implementation

#### Setting Up the Environment
The necessary development environment and libraries are set up using Visual Studio 2022 as the IDE. The setup process includes several steps:

1. **Installing Required Libraries:**
   - **GLFW:** Used to create an OpenGL window and handle input.
   - **GLEW:** Manages OpenGL function pointers.
   - **GLM:** Performs mathematical operations.
   - **ImGui:** Creates the graphical user interface due to its ease of use.

2. **Configuring Visual Studio:**
   - Add the include and lib directories for GLFW, GLEW, and ImGui to the project properties.
   - Set the paths to the include directories of these libraries under `Properties -> C/C++ -> General -> Additional Include Directories`.
   - Set the paths to the library directories under `Properties -> Linker -> General -> Additional Library Directories`.
   - Add `glew32s.lib`, `glfw3.lib`, `opengl32.lib`, `Shell32.lib`, `User32.lib`, and `Gdi32.lib` to the dependencies under `Properties -> Linker -> Input -> Additional Dependencies`.
   - Ensure that the project is set to use C++ version 17.

3. **Initializing Libraries:**
   - Include the GLEW header before the GLFW header to prevent initialization problems.
   - After creating the OpenGL context with GLFW, initialize GLEW.

#### Loading .off Files
Handling 3D model files includes:

1. **Downloading .off Files:**
   - Download 400 .off files from the [SEGAL Project at Princeton University](https://www.cs.princeton.edu/~min/seganalysis/).

2. **Implementing File Loading:**
   - Implement a function to load .off files, reading vertex and face data from the files and storing them in appropriate data structures.
   - Create structures to store vertices and faces of the models.

#### Setting Up ImGui for GUI
To provide user interaction, ImGui is integrated:

1. **Initializing ImGui:**
   - Initialize ImGui and create a window for transformation controls.

2. **Adding GUI Components:**
   - Add combo boxes to select .off files.
   - Include radio buttons and sliders for different types of transformations.
   - Add a reset button to revert transformations.

#### Applying Transformations on CPU
Various transformations are implemented on the CPU:

1. **Translation:**
   - Implement functions to move the model along the x, y, and z axes.

2. **Rotation:**
   - Add functions to rotate the model around the x, y, and z axes, as well as around an arbitrary axis.

3. **Scaling:**
   - Develop functions to scale the model with respect to a fixed point.

4. **Reflection and Shearing:**
   - Add functionalities to reflect the model over an arbitrary plane and apply shearing transformations in the x, y, and z directions.

#### Setting Up Shaders for GPU Rendering
To improve performance, transformations are moved to the GPU using shaders:

1. **Creating Shaders:**
   - Create vertex and fragment shaders. Vertex shaders handle the vertex transformations, while fragment shaders handle coloring and shading.

2. **Implementing Toon Shading:**
   - Implement toon shading in the fragment shader for a cartoon-like visual effect.

3. **Shader Programs:**
   - Set up shader programs and pass transformation matrices to the shaders.

#### GUI Enhancements
The GUI is enhanced to include options for different shading techniques:

1. **Adding Shading Options:**
   - Extend the ImGui interface to include options for switching between Toon Shading, Gooch Shading, and Phong Shading.
   - Add radio buttons for selecting the desired shading technique.

### Encountered Problems

1. **Integration Issues:**
   - **GLEW Initialization:** Initial issues with GLEW initialization were resolved by ensuring the correct order of header inclusion and initialization.
   - **Library Dependencies:** Correctly setting paths for GLFW, GLEW, and ImGui libraries in Visual Studio was challenging. Ensuring the right versions and paths were configured correctly was crucial.

2. **Performance Bottlenecks:**
   - **CPU-Based Transformations:** Transformations performed on the CPU were slow for large models. Moving to GPU-based transformations significantly improved performance.
   - **Rendering Optimization:** Initial rendering was inefficient, causing slow frame rates. Optimizations such as reducing state changes and minimizing draw calls improved performance.


### Running the Application

1. **Execute the Program:**
   - Run the compiled executable. The OpenGL window will open, displaying the 3D model.

2. **Using the GUI:**
   - Use the ImGui window to interactively apply transformations.
   - Select different .off files using the combo box.
   - Apply transformations using the sliders.
   - Reset transformations using the reset button.

### Additional Setup Notes
- Choose C++ version 17 in the Visual Studio project settings.
- The .off files used for testing can be downloaded from the [SEGAL Project at Princeton University](https://www.cs.princeton.edu/~min/seganalysis/).

