#! /usr/bin/python

import cgi, cgitb 

print "Content-Type: text/html\n\n"
print '<html><head><meta content="text/html; charset=UTF-8" />'
print '<title>Raspberry Pi</title><p>'


# Create instance of FieldStorage 
form = cgi.FieldStorage() 

# Get data from fields
q = form.getvalue('q')

if q=="wait":
   out_file = open("/tmp/wait","w")
   out_file.write("wait")
   out_file.close()

print q
print "</p></body></html>"
