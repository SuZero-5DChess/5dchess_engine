Structure of the code
========

```
  ┌──────────────┐                                             
  │ game         │                                 in stack    
  │     ╷  ╷ ... │                                             
  └─────┼──┼─────┘                                             
        │  └───────────────────┐                               
  ┌─────┴────────┐      ┌──────┴────────┐                      
  │ status 0     │      │ status 1      │  ...                 
  │     ╷        │      │      ╷        │                      
  └─────┼────────┘      └──────┼────────┘                      
        │                      │                               
  ┌─────┴────────┐      ┌──────┴────────┐                      
  │ multiverse 0 │      │ multiverse 1  │                      
  │     ╷╷╷      │      │    ╷╷╷╷       │                      
  └─────┼┼┼──────┘      └────┼┼┼┼───────┘                      
        |||                  |||└──────────┐
        │││   ┌──────────────┘│└─────┐     |       in heap     
        │││   │      ┌────────┴────┐ │     │                   
        │└┼───┼──────┤board (1T1)w │ │     │           ...     
        │ └───┼────┐ └─────────────┘ │     │                   
   ┌────┴─────┴─┐  │ ┌─────────────┐ │┌────┴───────┐           
   │board (0T0)b│  └─┤board (0T1)w ├─┘│board (0T1)b│   ...     
   └────────────┘    └─────────────┘  └────────────┘           
                                                               
                                                               
```                                                         
I try to follow the terminalogy described in <https://github.com/adri326/5dchess-notation>. (However, the engine does now support +0 and -0 timelines.) The classes defined are inspired from that.

+ A `game` object contains the current state and some historical states.

+ A `state` object contains a `multiverse` object and some additional information such as who is playing, if the player has submitted, etc. 

+ A `multiverse` object contains a 2-dimensional list of `board` pointers. Therefore, it is possible to reduce memory consumption by reusing board objects when the new multiverse is only partially different from the old one.

+ A `board` object contains a `piece_t` (which is equivalent to `unsigned char`) array of length 64. Thus the size of a board object is 64 bytes.

In the code, there are two coordinate systems: LTCXY (which is the coordinate for storing moves) and UVXY (which is the coordinate for indexing boards). In both systems, X and Y ranges from 0 to 7. The difference is: L can be positive or negative while U must be greater than or equal to zero. TC are two axes but V is just one axis. The functions `multiverse::l_to_u` `multiverse::tc_to_v` `multiverse::u_to_l` and `multiverse::v_to_tc` convert between these coordinates.

-----

For moves, the class `full_move` is basically a translation of this algebraic type:

```haskell
data FullMove = Submit | Move Vec4 Vec4
```

Initializer of `vec4` follows the order `(x,y,t,l)`. It supports addition, substraction, scalar multiplication and and comparation. I will talk more about the implementation of this class below.

Movegen
========

This is the python script used for generating code:
```python
from itertools import combinations, chain

def functions(domain, range):
    if len(domain) == 0:
        return [[]]
    else:
        x = domain.pop()
        fs = functions(domain, range)
        return [f+[(x,r)] for r in range for f in fs]

p = combinations(['x','y','t','l'], 4)
q = chain(*map(lambda x: functions(list(x), [1,-1]), p))

def show(f):
    for u in f:
        s = dict(u)
        for c in "xytl":
            s.setdefault(c, 0)
        print("vec4({x:2},{y:2},{t:2},{l:2})".format(**s), end=', ')

show(q)
```

Vector of four integers
=============

To implement `vec4` in a way that is very fast in addition, the program uses 
a trick from <https://stackoverflow.com/questions/79464417>. In `vec4.h`, it is
defined that the data of `vec4` is stored in a 32-bit integer, with
`X_BITS = Y_BITS = T_BITS = L_BITS = 8`, so valid values for each of them range
from -128 to 127. This means there can are maximally 256 timelines and
64 units of times allowed. However, the bits for each coordinate is in fact 
adjustable so long as `X_BITS` and `Y_BITS` are greater than `3` and
 `X_BITS + Y_BITS + T_BITS + L_BITS` is equal to 32.
 