//
//  VectorFont.h
//  PixelPyros
//
//  Created by Seb Lee-Delisle on 21/08/2012.
//
//

#pragma once 
#include "Letter.h"

class VectorFont {
	
	
public:
	
	VectorFont () {
		
		Letter A;
		A.addLine(0 , 6 , 0 , 1);
        A.addLine(0 , 1 , 2 , 0);
        A.addLine(2 , 0 , 4 , 1);
        A.addLine(4 , 1 , 4 , 6);
        A.addLine(4 , 3 , 0 , 3);

		
        letters["A"] = A;
        
        Letter A2(A);
        A2.addLine(1 , -0.5 , 1 , 0);
        A2.addLine(3 , -0.5 , 3 , 0);
        letters["ä"] = A2;
        letters["Ä"] = A2;
    
        Letter A3(A);
        A3.addLine(1, -2, 3, -1);
        letters["à"] = A3;
        letters["À"] = A3;
        
        Letter A4(A);
        A4.addLine(3, -2, 1, -1);
        letters["á"] = A4;
        letters["Á"] = A4;
       
		
		Letter B;
        B.addLine(0, 0, 0, 6);
		B.addLine(0, 6, 3, 6);
		B.addLine(3, 6, 4, 5);
		B.addLine(4, 5, 4, 4);
		B.addLine(4, 4, 3, 3);
		B.addLine(3, 3, 0, 3);
		B.addLine(0, 3, 3, 3);
		B.addLine(3, 3, 4, 2);
		B.addLine(4, 2, 4, 1);
		B.addLine(4, 1, 3, 0);
		B.addLine(3, 0, 0, 0);
        letters["B"] = B;
        
        Letter C;
		C.addLine(4, 0, 0, 0);
		C.addLine(0, 0, 0, 6);
		C.addLine(0, 6, 4, 6);
        
		letters["C"] = C;
		
        Letter D;
		D.addLine(0, 0, 3, 0);
		D.addLine(3, 0, 4, 2);
		D.addLine(4, 2, 4, 4);
		D.addLine(4, 4, 3, 6);
		D.addLine(3, 6, 0, 6);
		D.addLine(0, 6, 0, 0);
        
		letters["D"] = D;
        
        Letter E;
		E.addLine(4, 0, 0, 0);
		E.addLine(0, 0, 0, 6);
		E.addLine(0, 6, 4, 6);
		E.addLine(4, 3, 0, 3);
        
		letters["E"] = E;
        
        Letter E2(E);
        E2.addLine(1, -1, 3, -2);
        letters["é"] = E2;
        letters["É"] = E2;
        
        
        
        Letter F;
		F.addLine(4, 0, 0, 0);
		F.addLine(0, 0, 0, 6);
		F.addLine(0, 3, 4, 3);
        
        
		letters["F"] = F;
        
        Letter G;
		G.addLine(4, 1, 4, 0);
		G.addLine(4, 0, 0, 0);
		G.addLine(0, 0, 0, 6);
		G.addLine(0, 6, 4, 6);
		G.addLine(4, 6, 4, 3);
		G.addLine(4, 3, 2, 3);
        
        letters["G"] = G;
        
        Letter H;
		H.addLine(0, 0, 0, 6);
		H.addLine(0, 3, 4, 3);
		H.addLine(4, 0, 4, 6);
        
        letters["H"] = H;
        
        Letter I;
		I.addLine(0, 0, 4, 0);
		I.addLine(2, 0, 2, 6);
		I.addLine(0, 6, 4, 6);
        
        letters["I"] = I;
        
        //í
        Letter I2;
        I2.addLine(0, 0, 4, 0);
        I2.addLine(2, 0, 2, 6);
        I2.addLine(0, 6, 4, 6);
        I2.addLine(1, -1, 3, -2);
       
        letters["í"] = I2;
        letters["Í"] = I2;
        
        
        Letter J;
		J.addLine(0, 4, 1, 6);
		J.addLine(1, 6, 4, 6);
		J.addLine(4, 6, 4, 0);
		J.addLine(4, 0, 2, 0);
        
        letters["J"] = J;
        
        Letter K;
		K.addLine(0, 0, 0, 6);
		K.addLine(0, 3, 4, 0);
		K.addLine(0, 3, 4, 6);
        
        letters["K"] = K;
        
        Letter L;
        L.addLine(0 , 0 , 0 , 6);
        L.addLine(0 , 6 , 4 , 6);
        
        letters["L"] = L;
        
        Letter M;
		M.addLine(0, 6, 0, 0);
		M.addLine(0, 0, 2, 2);
		M.addLine(2, 2, 4, 0);
		M.addLine(4, 0, 4, 6);
        
        letters["M"] = M;
        
        Letter N;
		N.addLine(0, 6, 0, 0);
		N.addLine(0, 0, 0, 1);
		N.addLine(0, 1, 4, 5);
		N.addLine(4, 5, 4, 6);
		N.addLine( 4, 6, 4, 0);
        
        letters["N"] = N;
        
        Letter O;
		O.addLine(0, 0, 4, 0);
		O.addLine(4, 0, 4, 6);
		O.addLine(4, 6, 0, 6);
		O.addLine(0, 6, 0, 0);
        
        letters["O"] = O;
        
        Letter O2(O);
        O2.addLine(1 , -1 , 1 , -0.5);
        O2.addLine(3 , -1 , 3 , -0.5);
        letters["Ö"] = O2;
        letters["ö"] = O2;
        
        
        Letter P;
		P.addLine(0, 6, 0, 0);
		P.addLine(0, 0, 4, 0);
		P.addLine(4, 0, 4, 3);
		P.addLine(4, 3, 0, 3);
        
        letters["P"] = P;
        
        Letter Q;
		Q.addLine(0, 0, 4, 0);
		Q.addLine(4, 0, 4, 4);
		Q.addLine(4, 4, 2, 6);
		Q.addLine(2, 6, 0, 6);
		Q.addLine(0, 6, 0, 0);
		Q.addLine(2, 4, 4, 6);
        
        letters["Q"] = Q;
        
        Letter R;
		R.addLine(0, 6, 0, 0);
		R.addLine(0, 0, 4, 0);
		R.addLine(4, 0, 4, 3);
		R.addLine(4, 3, 0, 3);
		R.addLine(0, 3, 4, 6);
        
        letters["R"] = R;
        
        Letter S;
		S.addLine(4, 0, 0, 0);
		S.addLine(0, 0, 0, 3);
		S.addLine(0, 3, 4, 3);
		S.addLine(4, 3, 4, 6);
		S.addLine(4, 6, 0, 6);
        
        letters["S"] = S;
        
        Letter T;
		T.addLine(0, 0, 4, 0);
		T.addLine(2, 0, 2, 6);
        
        letters["T"] = T;
        
        Letter U;
		U.addLine(0, 0, 0, 5);
		U.addLine(0, 5, 1, 6);
		U.addLine(1, 6, 3, 6);
		U.addLine(3, 6, 4, 5);
		U.addLine(4, 5, 4, 0);
        letters["U"] = U;
        
        Letter U2(U);
        U2.addLine(1 , -1 , 1 , -0.5);
        U2.addLine(3 , -1 , 3 , -0.5);
        letters["ü"] = U2;
        letters["Ü"] = U2;
       
        
        
        Letter V;
		V.addLine(0, 0, 2, 6);
		V.addLine(2, 6, 4, 0);
        
        letters["V"] = V;
        
        Letter W;
		W.addLine(0, 0, 0, 6);
		W.addLine(0, 6, 2, 4);
		W.addLine(2, 4, 4, 6);
		W.addLine(4, 6, 4, 0);
        
        letters["W"] = W;
        
        Letter X;
		X.addLine(0, 0, 4, 6);
		X.addLine(4, 0, 0, 6);
		
        
        letters["X"] = X;
		
		Letter x;
		x.addLine(0, 0, 4, 6);
		x.addLine(4, 0, 0, 6);
		
		letters["x"] = x;
		
        Letter Y;
		Y.addLine(0, 0, 2, 2);
		Y.addLine(2, 2, 4, 0);
		Y.addLine(2, 2, 2, 6);
        
        letters["Y"] = Y;
        
        Letter Z;
		Z.addLine(0, 0, 4, 0);
		Z.addLine(4, 0, 0, 6);
		Z.addLine(0, 6, 4, 6);
        
        letters["Z"] = Z;
        
        Letter l0;
		l0.addLine(0, 6, 4, 0);
		l0.addLine(4, 0, 0, 0);
		l0.addLine(0, 0, 0, 6);
		l0.addLine(0, 6, 4, 6);
		l0.addLine(4, 6, 4, 0);
        
        letters["0"] = l0;
        
        Letter l1;
		l1.addLine(0, 0, 2, 0);
		l1.addLine(2, 0, 2, 6);
		l1.addLine(0, 6, 4, 6);
        
        letters["1"] = l1;
        
		// only fixed up to this point
        Letter l2;
        l2.addLine(0 , 0 , 4 , 0);
        l2.addLine(4 , 0 , 4 , 3);
        l2.addLine(4 , 3 , 0 , 3);
        l2.addLine(0 , 3 , 0 , 6);
        l2.addLine(0 , 6 , 4 , 6);
        
        letters["2"] = l2;
        
        
        Letter l3;
        l3.addLine(0 , 0 , 4 , 0);
        l3.addLine(4 , 0 , 4 , 6);
        l3.addLine(0 , 3 , 4 , 3);
        l3.addLine(0 , 6 , 4 , 6);
        
        letters["3"] = l3;
        
        Letter l4;
        l4.addLine(0 , 0 , 0 , 3);
        l4.addLine(0 , 3 , 4 , 3);
        l4.addLine(4 , 0 , 4 , 6);
        
        
        letters["4"] = l4;
        
        Letter l5;
//        l5.addLine(0 , 0 , 0 , 0);
//        l5.addLine(0 , 0 , 4 , 0);
//        l5.addLine(0 , 0 , 0 , 3);
//        l5.addLine(0 , 3 , 4 , 3);
//        l5.addLine(4 , 3 , 4 , 6);
//        l5.addLine(0 , 6 , 4 , 6);
		l5.addLine(4, 0, 0, 0);
		l5.addLine(0, 0, 0, 3);
		l5.addLine(0, 3, 4, 3);
		l5.addLine(4, 3, 4, 6);
		l5.addLine(4, 6, 0, 6);
		
        letters["5"] = l5;
        
        Letter l6;
        l6.addLine(0 , 0 , 4 , 0);
        l6.addLine(0 , 0 , 0 , 6);
        l6.addLine(0 , 3 , 4 , 3);
        l6.addLine(4 , 3 , 4 , 6);
        l6.addLine(0 , 6 , 4 , 6);
        
        letters["6"] = l6;
        
        
        Letter l7;
        l7.addLine(0 , 0 , 4 , 0);
        l7.addLine(4 , 0 , 4 , 6);
        
        letters["7"] = l7;
        
        Letter l8;
        l8.addLine(0 , 0 , 4 , 0);
        l8.addLine(4 , 0 , 4 , 3);
        l8.addLine(4 , 3 , 0 , 3);
        l8.addLine(0 , 3 , 0 , 0);
        l8.addLine(4 , 6 , 4 , 3);
        l8.addLine(0 , 3 , 0 , 6);
        l8.addLine(0 , 6 , 4 , 6);
        
        letters["8"] = l8;
        
        
        Letter l9;
        l9.addLine(0 , 0 , 0 , 3);
        l9.addLine(0 , 3 , 4 , 3);
        l9.addLine(0 , 0 , 4 , 0);
        l9.addLine(4 , 0 , 4 , 6);
        
        letters["9"] = l9;
        
        Letter lex;
        lex.addLine(2 , 0 , 2 , 4);
        lex.addLine(2 , 5.5 , 2 , 6);
        letters["!"] = lex;
        
        Letter lcol;
        lcol.addLine(2 , 1 , 2 , 3);
        lcol.addLine(2 , 4 , 2 , 6);
        letters[":"] = lcol;
        
        Letter lper;
        lper.addLine(2 , 5 , 2 , 6);
        letters["."] = lper;
        
        Letter lhypen;
        lhypen.addLine(1 , 3 , 3 , 3);
        letters["-"] =lhypen;
        
        Letter lhash;
        lhash.addLine(1 , 1 , 1 , 5);
        lhash.addLine(0 , 2 , 4 , 2);
        lhash.addLine(0 , 4 , 4 , 4);
        lhash.addLine(3 , 1 , 3 , 5);
        letters["#"] =lhash;
        
        Letter lcomma;
        lcomma.addLine(2 , 5 , 2 , 6);
        letters[","] =lcomma;
        
        Letter lfslash;
        lfslash.addLine(0 , 6 , 4 , 0);
        letters["/"] =lfslash;
		
		Letter lfqmark;
		lfqmark.addLine(0 , 0 , 4 , 0);
        lfqmark.addLine(4 , 0 , 4 , 3);
        lfqmark.addLine(4 , 3 , 2 , 3);
        lfqmark.addLine(2 , 3 , 2 , 4);
        lfqmark.addLine(2 , 5 , 2 , 6);
	    letters["?"] =lfqmark;
		
        Letter dblQuote;
        dblQuote.addLine(1,0,1,2);
        dblQuote.addLine(3,0,3,2);
        letters["\""] = dblQuote;
        letters["“"] =dblQuote;
        letters["”"] =dblQuote;
        
        Letter sQuote;
        sQuote.addLine(3,0,3,2);
        letters["'"] = sQuote;
        letters["‘"] =sQuote;
        letters["’"] =sQuote;
        
        Letter lbrack;
        lbrack.addLine(3,0,1,0);
        lbrack.addLine(1,0,1,6);
        lbrack.addLine(1,6,3,6);
        letters["["] = lbrack;
        letters["("] = lbrack;
        
        Letter rbrack;
        rbrack.addLine(1,0,3,0);
        rbrack.addLine(3,0,3,6);
        rbrack.addLine(3,6,1,6);
        letters["]"] = rbrack;
        letters[")"] = rbrack;
        
        Letter atsign;
        atsign.addLine(2,3,2,4);
        atsign.addLine(2,4,4,4);
        atsign.addLine(4,4,4,0);
        atsign.addLine(4,0,0,0);
        atsign.addLine(0,0,0,6);
        atsign.addLine(0,6,4,6);
        letters["@"] = atsign;
        
	}
    
    map <string, Letter> letters;
};
