HelpMsg         db      'Na?te program nad prvn?ch 64 kB pam?ti a pak ho spust?.',0dh,0ah,0dh,0ah
                db      'LOADFIX [jednotka:][cesta]soubor',0dh,0ah,0dh,0ah
                db      'Pou?ijte LOADFIX k na?ten? programu, pokud obdr??te zpr?vu',0dh,0ah
                db      '"Packed file corrupt" p?i pokusu o na?ten? do doln? pam?ti.',0dh,0ah
                db      '$'

ErrMsg  db      'Chyba p?i spou?t?n? dan?ho programu','$'
NoParms db      'Chyba: Nebyl zad?n n?zev programu','$'
NoComspec       db      'Chyba: chyb? prom?nn? COMSPEC','$'


