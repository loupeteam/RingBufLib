# Info
Library is provided by Loupe  
https://loupe.team  
info@loupe.team  
1-800-240-7042  

# Description
This library implements a Ring buffer. The user may add/remove items to the top or bottom of the buffer.  
Values can be retrieved by giving the index of the value relative to the top of the buffer. Index 0 corresponds to the top of the buffer.  
If more values are added than there is memory allocated, the item furthest from the end which the new value is add, bottom or top, will be dropped from the buffer.

For more documentation and examples, see https://loupeteam.github.io/LoupeDocs/libraries/ringbuflib.html

# Installation
To install using the Loupe Package Manager (LPM), in an initialized Automation Studio project directory run `lpm install ringbuflib`. For more information about LPM, see https://loupeteam.github.io/LoupeDocs/tools/lpm.html

## Licensing

This project is primarily licensed under the [MIT License](LICENSE). 