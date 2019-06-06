# Qentem Engine (v0.7.1)

## Summary:
Qentem is a stand-alone library, built to provide a simple way to match existing syntaxes or new ones. It also provides call-backs for post or per-parsing. Built for templates, then rewritten for general-propose matching of text or syntaxes - Regular expression on steroids.

## The project:
The project is about the algorithm not the code. That's why it's written in a as-simple-as-possible form, which allowed it to be compiled easily to Web Assembly without any modifications.

## The code:
Qentem is built to be ported to any managed language, so I tried to limit the use of C++ specific syntax. The code itself was ported from PHP. It has no try-catch statements nor Goto. Also, I wanted to use some assembly code, but that would drastically limit it from being ported to other languages.

## Status:
The project is in early development stage. However, the engine has been rewritten over and over to ensure it delivers the expected results, which is achieved through extensive tests (3 times the size of the engine). It's working and has a full implementation of HTML template and JSON.
