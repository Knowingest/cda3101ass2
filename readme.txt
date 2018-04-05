Harrison Hill
project 2
CDA3101
blahblahblah


This shit is pretty unfinished but lets be honest it won't be the worst one you see today.

My program reads in and runs instructions through a pipeline successfully.
The only instructions that give their desired results are ORI, ADDI, LW, and SW.

The others just run through without doing anything... except giving wackadoo pipeline register values.

A couple of the pipeline registers have wacky values but the majority are correct.

I checked my work against the NoHazards.exe given to us, and using vimdiff there's just a couple
problem pipeline registers that are off.  The Memory and register files are correct.

my R I G O R O U S test suits includes hhilltest1.asm, which is 100 ori commands and some loose data words,
and hhilltest2.asm, which just shows some ori andi and lw sw commands.

The 100 commands plus data just shows that I successfully can read in a program of that size.
	(and that I'm not just using an array of size 100, since there are actually ~130 lines of input)

The other test shows that LW and Sw actually work.


......

THERES NOT A LOT TO TEST OK


______

I've been feeling pretty down lately.

My friend told me "It could always be worse, You could be stuck in a hole filled with water."

I know he means well.

