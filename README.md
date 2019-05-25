# Qentem Engine (v0.5.0)
General purpose syntax parser.

## Summary:
Qentem is a stand-alone library, built to provide a simple way to match existing syntax or new ones. It also provides call-backs for post or per-parsing. Built for templates, then rewritten for general-propose matching of text or syntaxes.

## Background:
This project was stated to provide a fast way to render templates in PHP and using regular expressions or eval is very expensive operation, not to mention the security risks the involves executing a code at the script privilege level, or even execute binary files.

The idea came from a JavaScript code that was develop as to solve nested tags/matching in a math operation: (2-1)*(5(2/2)). Regular expression was uses in a recursive function to extract the inner () first and preform the math, then replace the result and so on until there is only numbers left.

The JavaScript code was then adopted as template engine for PHP, but the limitation appeared when matching nested if-statements. Rewritten over and over to remove the need for regular expressions, then performance started to justify the work of using custom code instead of regex.

But after moving the code base to C (then C++ later) the speed was 1:1000 of what it used to be on PHP. That opened it to unlimited ways of using the new engine, and so, Qentem is used as a name to present an easy and new way of dealing with nested matching, and to describe the mechanism that is being used in this engine.

## Status:
The project is in a very early stage. However, the engine has been rewritten over and over to ensure it delivers the desire results, which is achieved through extensive tests (3 times the size of the engine). It's working and has a full implementation of HTML template. Also, it has an early implementation for JSON and other syntaxes. Still, the engine does what it should, and the rest is just publishing and add-ons, as the code was built from the ground-up to be module.

To be fair, the HTML Template is actually a HTML JET Engine; since it can do nested if-else statements, nested loops, variables, and has an ALU unit functioning inside if cases/conditions.

## Present:
I had to stop working on this project to focus on my studies, and now that I have some time, the work will – hopefully – resume.

## Future:
Moving closer to the metal provided extreme control over every operation, so hopefully, the base of the engine will get implemented on a chip. Especially when it's under 500 lines of code (the base code).

## The code:
It was built to be ported to any managed language, so I tried to limit the use of C/C++ specific syntax. The code itself was ported from PHP. It has no try-catch statements nor Goto. Also, I wanted to use some assembly code, but that would drastically limit it from being ported to other languages.

## Qentem, The algorithm:
The entire project is about the algorithm and never about the language. That why it's written in a as-simple-as-possible form. Which allowed it to be compiled easily to Web Assembly without any modifications.

## In a word:
Regular expression on steroids.

## Q: Why most of the code has no comments?
A: Well, I had one and half week to port it to C, then C++. Then I had to rewrite most of it in another 2 weeks. And had only two weeks to bug the bugs out. After reaching v0.5.0, my old laptop broke (the poor thing couldn't handle the heat any more), and since the project was on a rapid development stage, I was holding off the comments for the next release, as I wasn't sure if it will actually work or not (it does now), then the semester started, and had to focus on my studies.

## Fun fact:
The most annoying syntax that I had to deal with is HTML, since having a second tag is optional and only has 2 characters to match (starts with < and end with > or ></>). not much to match for and at the same time it's complicated to do. However, solving it stabilized the entire engine. Also, it’s the result of 10+ (side project) of designing something that – finally – provides a seamless experience when interacting with existing syntaxes, or custom ones.

## Code Owner:
Just a man who code for fun and money
