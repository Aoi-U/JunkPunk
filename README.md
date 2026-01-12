#Setup
1. Open the Game project properties
<img width="447" height="806" alt="image" src="https://github.com/user-attachments/assets/90475b9e-9121-479c-89a0-18e6c4849549" />

2. In the window that pops up, add "$(ProjectDir)thirdparty\include folder" in C/C++ -> General -> Additional Include Directories
<img width="800" height="542" alt="image" src="https://github.com/user-attachments/assets/06036dc5-379b-4ac7-bb9b-46677c7e2396" />

3. Add "$(ProjectDir)thirdparty\lib in Linker -> General -> Additional Library Directories
<img width="802" height="545" alt="image" src="https://github.com/user-attachments/assets/dc2ceed3-7396-4cce-8108-53998939debf" />

4. Add glfw3.lib, assimp-vc143-mt.lib in Linker -> Input -> Additional Dependencies dropdown menu
<img width="799" height="547" alt="image" src="https://github.com/user-attachments/assets/ac722239-fcdd-41b7-8895-0df0927fea72" />

5. Apply and exit
  
6. Make sure you are on Release or Debug and on x64
<img width="389" height="55" alt="image" src="https://github.com/user-attachments/assets/b83b87e6-0314-4a9f-9829-8dfd97857903" />
