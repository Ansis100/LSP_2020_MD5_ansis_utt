# Eseja "Dinamiskās atmiņas izdalīšanas algoritmu veiktspēja"
## Komandas dalībnieki un to ieguldījums
Ģirts Ratnieks = 25%  
Krišjānis Veinbahs = 25%  
Ansis Spruģevics = 25%  
Andris Lapiņš = 25%  

## Algoritmi
### Best fit algoritms
Best fit algoritms brīvo atmiņu pārbauda, salīdzinot brīvo bloku izmērus pret pieprasīto izmēru.
Pieprasītā vieta tiek alocēta tajā blokā, kurā paliks pāri vismazākais daudzums vietas, lai pēc iespējas vairāk aizpildītu brīvo bloku.
Šāda darbība rezultējas ar daudz sīkiem blokiem (1-5B), kuri netiek aizņemti.

### Worst fit algoritms
Worst fit algoritms alocē atmiņu katram alokācijas pieprasījumam meklējot
lielāko iespējamo atmiņas fragmentu un rezervējot vietu šī lielākā atmiņas
fragmenta sākumā.

### First fit algoritms
First fit atmiņas alokācijas shēma pārbauda tukšos atmiņas blokus secīgi, tas nozīmē, ka tukšajiem atmiņas blokiem, kuri tiek atrasti pirmie, sākumā pārbauda izmēru, ja izmērs ir lielāks, tad to allocē. Lielākā problēma šajā allocēšanas shēmā ir tā, ka, kad process tiek allocēts izteikti lielākā atmiņas blokā, tas sataisa lielus chunkus, kas paliek pāri.

### Next fit algoritms

## Mērījumi, eksperimenti
### Algoritmu fragmentācija un ātrdarbība
Best Fit vidējais ātrums - 11 CPU taktis
Worst Fit vidējais ātrums - 47.5 CPU taktis
First Fit vidējais ātrums - 41 CPU taktis
Next Fit vidējais ātrums - 4 CPU taktis

Best Fit vidējā fragmentācija - 71%
Worst Fit vidējā fragmentācija - 85%
First Fit vidējā fragmentācija - 79%
Next Fit vidējā fragmentācija - 81%

## Secinājumi
Algoritmi savā darbībā ir diezgan līdzvērtīgi, bet katram ir citāds pielietojums.
Šī koda gadījumā ir diezgan lielas atšķirības algoritmu ātrdarbībā, jo katra algoritma implementāciju ir rakstījuši dažādi cilvēki, tāpēc koda optimizācija ir atšķirīga.
Algoritmu fragmentācijas aprēķinos tika izmantota viena formula, tāpēc vidējos fragmentācijas rādītājus var salīdzināt starp algoritmiem.

Šo testu gadījumā algoritmi pēc fragmentācijas ierindojas šādā secībā (dilstoši pēc efektivitātes):
- Best Fit
- First Fit
- Next Fit
- Worst Fit
