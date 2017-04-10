# islandora_datastreams_io
This module is a web-ui wrapper for the islandora_datastream_crud module.

Since this module shells out to the drush commands from islandora_datastream_crud and that module
prompts the user for some operations such as creating folders, overwriting files, other reasons - 
that module had to be forked so that a new parameter could be handled that would run the command 
without any prompting.

TODO: This module will need to extend the options based on what is installed.  In other words, if 
islandora_solr is installed, provide the option to import / export using a Solr query... if the 
upitt_workflow module is installed, provide the option to import / export using a batch.

The PID values can be passed from other modules by setting the $_SESSION['pids'] value before 
redirecting to the islandora/datastreams_io/export page.  Doing this will make the PIDS field 
read-only and set the "Select Objects" mode to "List of PID values".

