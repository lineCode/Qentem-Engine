# Qentem Engine (v0.6.3)

## Summary:
Qentem is a stand-alone library, built to provide a simple way to match existing syntaxes or new ones. It also provides call-backs for post or per-parsing. Built for templates, then rewritten for general-propose matching of text or syntaxes - Regular expression on steroids.

## Background:
This project was stated to provide a fast way to render templates in PHP and using regular expressions or "eval" is very expensive operation, not to mention the security risks the involves executing a code at the script privilege level, or even execute binary files.

The idea came from a JavaScript code that was develop as to solve nested tags/matching in a math operation: (2-1)*(5(2/2)). Regular expression was used in a recursive function to extract the inner () first and preform the math, then replace the result and so on until there is only one number's left.

The JavaScript code was then adopted as template engine for PHP, but the limitation appeared when matching nested if-statements. Rewritten over and over to remove the need for regular expressions, then performance started to justify the work of using custom code instead of regex.

But after moving the code base to C (then C++ later) the speed was 1:1000 of what it used to be on PHP. That opened it to unlimited ways of using the new engine, and so, Qentem is used as a name to present an easy and new way of dealing with nested matching, and to describe the mechanism that is being used in this engine.

## The code:
Qentem is built to be ported to any managed language, so I tried to limit the use of C++ specific syntax. The code itself was ported from PHP. It has no try-catch statements nor Goto. Also, I wanted to use some assembly code, but that would drastically limit it from being ported to other languages.

## The algorithm:
The project is about the algorithm not the code. That's why it's written in a as-simple-as-possible form, which allowed it to be compiled easily to Web Assembly without any modifications.

## Status:
The project is in early development stage. However, the engine has been rewritten over and over to ensure it delivers the expected results, which is achieved through extensive tests (3 times the size of the engine). It's working and has a full implementation of HTML template and JSON.
