HelpMsg         db      '?aduje program powy?ej pierwszych 64 K pami?ci i uruchamia go.',0dh,0ah,0dh,0ah
                db      'LOADFIX [dysk:][?cie?ka]nazwa_pliku',0dh,0ah,0dh,0ah
                db      'U?yj LOADFIX do ?adowania programu, je?li podczas pr?by ',0dh,0ah
                db      '?adowania programu do pami?ci niskiej pojawia si? komunikat ',0dh,0ah
                db      '"Uszkodzenie pliku skompresowanego".',0dh,0ah
		db	'$'

ErrMsg  db      'B??d wykonywania podanego programu','$'
NoParms db      'B??d: brak programu o podanej nazwie','$'
NoComspec       db      'B??d: brak COMSPEC','$'


