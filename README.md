# IOS2
Synchronisation of processes 

Implementujte v jazyce C modifikovaný synchronizační problém Faneuil Hall Problem (mužete se inspirovat knihou The Little Book of Semaphores).
Existují dva druhy vláken: (immigrants) a jeden soudce (judge). Pristehovalci musí cekat ve fronte, vstoupit do soudní budovy, zaregistrovat se a pak cekat na rozhodnutí soudce. V urcitém okamžiku vstoupí soudce do budovy. Když je soudce v budove, nikdo jiný nesmí vstoupit dovnitr ani odejít z budovy. Jakmile se všichni pristehovalci, kterí vstoupili do budovy, zaregistrují, muže soudce vydat rozhodnutí (potvrdit naturalizaci). Po vydání rozhodnutí (potvrzení) si pristehovalci vyzvednou certifikát o obcanství USA. Soudce odejde z budovy v urcitém okamžiku po rozhodnutí. Poté, co pristehovalci vyzvednou certifikát, mohou odejít.
Pristehovalci vstupují do budovy jednotlive (pouze jeden turniket) a také se jednotlive registrují (pouze jedno registracní místo). Soudce vydává rozhodnutí pro všechny registrované pristehovalce naráz. Certifikáty si imigranti vyzvedávají nezávisle na sobe a prítomnosti soudce v budove.

