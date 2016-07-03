# iTunes Orphanator
This tool is very useful when maintaining your iTunes library.
It's made for the Windows version of iTunes.

**Main features**
- Find orphan files, files in iTunes Media folder but not referenced in the iTunes library.
- Dead links, files in itunes, but not in iTunes Media.
- The tool also exports the two lists used in the comparison:
        - Listed, all files in the library.
        - Existing, all files on disk.

### How it works
It does not take any arguments. Instead it expects the standard Windows iTunes directory layout. And that you've exported the library to an XML.
This is simply performed by *File->Library->Export*.

Export it to:
`QDir::homePath()+"\Music\iTunes\Library.xml`

The directory should look like this (by default):
<pre>
# QDir::homePath() *%userprofile%*
  └─ Music
     └─ iTunes
        ├─ Album Artwork
        ├─ iTunes Media
        ├─ Mobile Applications
        ├─ Previous iTunes Libraries
        ├─ iTunes Library.itl
        ├─ ...
        └─ <b>Library.xml</b> (your export)
</pre>

When done, there will be 4 files made in the execution directory.
| Files        				| Content 										|
| ------------------------- | --------------------------------------------- |
| Orphanator_existing.txt	| All files on disk.				 			|
| Orphanator_listed.txt		| All files referenced by iTunes. 				|
| Orphanator_orhpans.txt	| Files on disk, but not referenced by iTunes. 	|
| Orphanator_unref.txt		| Files referenced by iTunes, but not on disk. 	|

That it really. Quickly made. Used once.

No further development planned until iTunes messed up the library again.
