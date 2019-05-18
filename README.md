The goal of this project is to create a useable English morphological analyser for the [Alice project](https://github.com/r0ller/alice/wiki).  

The whole project is based on the following resources:  

University of Pennsylvania, XTAG Project: https://www.cis.upenn.edu/~xtag/    

morph-1.5: https://www.cis.upenn.edu/~xtag/swrelease.html  

original text file: morph-1.5\data\morph_english.flat  

I just commited all intermediate artifacts (source code, db files, fst, build scripts, etc.) emerged during the conversion of the text file morph_english.txt.  

Some short description:  
-createdb.sql was used the create the empty db files  
-upenn_morphtxt2db.cpp was used to create the converted_engmorph.db from comment_free_morph_english.txt  
-adjust_upenn_morphdb.cpp was used to adjust it and get engmorph.db which was finally used onwards  
-the various build*.sh scripts were used to build the programs to convert the relevant info from the engmorph.db into the corresponding *.lexc files  
-english.foma was taken from the [foma](https://fomafst.github.io/morphtut.html) site and enhanced manually
-finally some manual adjustments were applied to the lexc files  
-createfst.sh was used to create english.fst in the end  

Note: This is not a 1:1 transformation of the original and is still in development so expect many bugs/mistakes  
