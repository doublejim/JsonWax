# JsonWax
JsonWax is a Qt C++ library for handling JSON-documents. It's an alternative to Qt's built-in set of JSON classes.

The purpose is to shorten your JSON-handling code, and keep your mind on the structure of your document.

With JsonWax, you do not need to think about JSON-Arrays, JSON-Objects and JSON-Values. Essentially, finding a value in a JSON-document is like finding a file - where the path is a sequence of strings and/or numbers, expressed as a QVariantList.

It is possible to write the whole "directory path" in one line, which makes nested JSON-documents easy to manage.

I have created easy-to-use functions for common operations such as: Loading the document from a file, saving to a file, and copying/moving data from one part of the document to another (or to another document!).

Unfortunately this ease of use comes at the price of being slower than QJsonDocument ( ~ 3.3 times slower according to my tests - currently I'm only comparing the parse speed.) 

You may use JsonWax under any of these licenses:

GNU General Public License version 2.0       https://www.gnu.org/licenses/gpl-2.0.html

GNU General Public License version 3         https://www.gnu.org/licenses/gpl-3.0.html

*** Could still use some more tests, however it is currently in working condition ***

See documentation on the website:

Website: https://doublejim.github.io/
