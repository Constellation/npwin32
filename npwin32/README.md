NPAPI DLL for calling DLL functions inside JavaScript.
======================================================

Sample
------
	<embed type="application/x-win32api-dynamic-call" id="p" hidden="true" />
	<script type="text/javascript">
	function foo(){
	    try{
	        var plugin = document.getElementById( "p" ); // get plugin instance
	        var GetWindowsDirectory = plugin.import( "kernel32.dll", "GetWindowsDirectoryW", "WN", "N" );
	        var MessageBox = plugin.import( "user32.dll", "MessageBoxW", "NWWN", "N" );
	        var buf = new Array( 257 ).join( " " ); // white space * 256

	        if( GetWindowsDirectory && MessageBox ){
	            GetWindowsDirectory.call( buf, buf.length );
	            MessageBox.call( 0, "Windows directory is '" + GetWindowsDirectory.arg(0) + "'", "Chrome", 0 );
	        }
	    }
	    catch( e ){
	        alert( e );
	    }
	}
	</script>
    

License
-------
the MIT License


Author
------
Yosuke HASEGAWA  [utf-8.jp](http://utf-8.jp/)




