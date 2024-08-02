# 1DPong

This game was made as a C++ program with OpenGL2 for a 7 day experimental gameplay jam (theme "minimalism") in 2009.

![image](https://github.com/user-attachments/assets/cfeb2711-f347-4969-a757-4b12668f4328)

The game obviously suits the minimalist theme being 1D Pong.  A friend and I had an in-joke where we'd play "1D Pong" with some of the lab equipment, the absurdity being that no one could win.  My challenge was coming up with a win condition while keeping the gameplay 1-dimensional, plus the show-off irony of doing a 3D game with 1D gameplay.

I got a nice review of the game in my inbox, which was the first time I'd gotten any kind of feedback on a game I'd made from a stranger:
![image](https://github.com/user-attachments/assets/cb6cf7b4-626c-431e-b92a-993121ab0ec0)

I reworked some graphics demo code I had before to make the game, and used another friend's neat template metaprogramming thing for physics unit matching.

The most interesting thing technically I did was the AI.

I had just read about some basics about function fitting / machine learning.

So I make the assumption that for a simple impulse function like this where the AI has some amount of state and a single output (rather than a high level planning type AI) you can model it as a polynomial and I used least squares fitting.

So for example if you had one input you could model the impulse function as the (in this case 2nd order) polynomial
f(x) = a + bx + cx^2

if you had 2 inputs: 
f(x,y) = a + bx + cy + dxy + ex^2 + fy^2

and so forth.  

I then "trained" the AI by using a least squares matrix to fit the polynomial coefficients to myself playing the game against the AI, periodically sampling the game state.  At first the AI did nothing so I just moved the paddle back and forth.  As the AI learned more and more tricks I basically "bootstrapped" the AI by training it using myself playing the game.


