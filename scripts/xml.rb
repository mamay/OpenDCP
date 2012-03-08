#!/usr/bin/ruby

require 'rubygems'
require 'xmlsimple'

f = File.open("f.txt") 
t = []

f.each_line do |line|
  t.push line.chomp!
end

xml = XmlSimple.xml_in('opendcp_de.ts')

x = 0
xml['context'].each do |i|
  i['message'].each do |j|
    j['translation'][0] = t[x]
    x = x + 1
  end
end

root = "TS"
dec = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"

File.open('x.ts','w') do |file|
   file.puts XmlSimple.xml_out(xml, { 'RootName' => 'TS', 
                                      'XmlDeclaration' => dec,
                                      'Indent' => '    '
                                    }) 
end
