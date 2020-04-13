/*
 * JOOS is Copyright (C) 1997 Laurie Hendren & Michael I. Schwartzbach
 *
 * Reproduction of all or part of this software is permitted for
 * educational or research use on condition that this copyright notice is
 * included in any copy. This software comes with no warranty of any
 * kind. In no event will the authors be liable for any damages resulting from
 * use of this software.
 *
 * email: hendren@cs.mcgill.ca, mis@brics.dk
 */

/* iload x        iload x        iload x
 * ldc 0          ldc 1          ldc 2
 * imul           imul           imul
 * ------>        ------>        ------>
 * ldc 0          iload x        iload x
 *                               dup
 *                               iadd
 */

int simplify_multiplication_right(CODE **c)
{
	int x, k;
	if (is_iload(*c, &x) &&
		is_ldc_int(next(*c), &k) &&
		is_imul(next(next(*c))))
	{
		if (k == 0)
			return replace(c, 3, makeCODEldc_int(0, NULL));
		else if (k == 1)
			return replace(c, 3, makeCODEiload(x, NULL));
		else if (k == 2)
			return replace(c, 3, makeCODEiload(x, makeCODEdup(makeCODEiadd(NULL))));
		return 0;
	}
	return 0;
}

/* 
 * ldc 0            ldc 1           ldc 2
 * iload x          iload x         iload x
 * imul             imul            imul
 * ------>          ------>         ------>
 * ldc 0            iload x         iload x
 *                                  dup
 *                                  iadd
 */

int simplifyMultiplicationLeft(CODE **c) 
{
    int x, k;
    if (is_ldc_int(*c, &k) && 
    	is_iload(next(*c), &x) && 
    	is_imul(next(next(*c))))
    {
        if (k == 0)
            return replace(c, 3, makeCODEldc_int(0, NULL));
        else if (k == 1) 
            return replace(c, 3, makeCODEiload(x, NULL));
        else if (k == 2)
            return replace(c, 3, makeCODEiload(x, makeCODEdup(makeCODEiadd(NULL))));
        return 0;
    }
    return 0;
}

/* 
 * iload x
 * ldc 1
 * idiv
 * -------->
 * iload x
 */

int simplifyDivision(CODE **c) {
    int x, k;
    if (is_iload(*c, &x) && 
    	is_ldc_int(next(*c), &k) && 
    	is_idiv(next(next(*c))) &&
    	k == 1) 
        return replace(c, 3, makeCODEiload(x, NULL));
    return 0;
}

/*
 * iload x
 * ldc 0
 * iadd
 * -------->
 * iload x
 */

int simplifyAddition(CODE **c) 
{
    int x, k;
    if (is_iload(*c, &x) && 
    	is_ldc_int(next(*c), &k) && 
    	is_iadd(next(next(*c))) &&
    	k == 0) 
        return replace(c, 3, makeCODEiload(x, NULL));
    return 0;
}

/*
 * iload x
 * idc 0
 * isub
 * -------->
 * iload x
 */

int simplifySubtraction(CODE **c) {
    int x, k;
    if (is_iload(*c, &x) && 
    	is_ldc_int(next(*c), &k) && 
    	is_isub(next(next(*c))) &&
    	k == 0)
        return replace(c, 3, makeCODEiload(x, NULL));
    return 0;
}

/* dup 			dup
 * istore x 	astore x
 * pop 			pop
 * --------> 	-------->
 * istore x 	astore x
 */
int simplifyStore(CODE **c)
{
	int x;
	if (is_dup(*c) &&
		is_istore(next(*c), &x) &&
		is_pop(next(next(*c))))
	{
		return replace(c, 3, makeCODEistore(x, NULL));
	}

	if (is_dup(*c) &&
		is_astore(next(*c), &x) &&
		is_pop(next(next(*c))))
	{
		return replace(c, 3, makeCODEastore(x, NULL));
	}
	return 0;
}

/* 
 * astore k 		istore k
 * aload k 			iload k
 * --------->  		--------->
 * dup   			dup
 * astore k 		istore k
 */

int simplifyStoreLoad(CODE **c)
{ 
	int k1, k2;
  	/* istore, iload */
	if (is_istore(*c, &k1) && 
		is_iload(next(*c), &k2) &&
		k1 == k2) {
		return replace(c, 2, makeCODEdup(makeCODEistore(k1, NULL)));
	}

  	/* astore, aload */
  	if (is_astore(*c, &k1) && 
  		is_aload(next(*c), &k2) && 
  		k1 == k2) {
    	return replace(c, 2, makeCODEdup(makeCODEastore(k1, NULL)));
  	}

  	return 0;
}

/* iload x
 * ldc k   (0<=k<=127)
 * iadd
 * istore x
 * --------->
 * iinc x k
 */
int positive_increment(CODE **c)
{
	int x, y, k;
	if (is_iload(*c, &x) &&
		is_ldc_int(next(*c), &k) &&
		is_iadd(next(next(*c))) &&
		is_istore(next(next(next(*c))), &y) &&
		x == y && 0 <= k && k <= 127)
	{
		return replace(c, 4, makeCODEiinc(x, k, NULL));
	}
	return 0;
}

/* iload x
 * ldc k   (0<=k<=127)
 * iadd
 * istore x
 * --------->
 * iinc x k
 */
int negative_increment(CODE **c)
{
	int x, y, k;
	if (is_iload(*c, &x) &&
		is_ldc_int(next(*c), &k) &&
		is_iadd(next(next(*c))) &&
		is_istore(next(next(next(*c))), &y) &&
		x == y && -127 <= k && k <= 0)
	{
		return replace(c, 4, makeCODEiinc(x, k, NULL));
	}
	return 0;
}

/* goto L1
 * ...
 * L1:
 * goto L2
 * ...
 * L2:
 * --------->
 * goto L2
 * ...
 * L1:    (reference count reduced by 1)
 * goto L2
 * ...
 * L2:    (reference count increased by 1)  
 */
int simplify_goto_goto(CODE **c)
{
	int l1, l2;
	if (is_goto(*c, &l1) && is_goto(next(destination(l1)), &l2) && l1 > l2)
	{
		droplabel(l1);
		copylabel(l2);
		return replace(c, 1, makeCODEgoto(l2, NULL));
	}
	return 0;
}

/* iconst_0             iconst_0
 * if_icmpeq label      if_icmpne label
 * --------->           --------->
 * ifeq label           ifeq label
 */
int simplifyEqualityComparison(CODE **c)
{
	int x;
	int l;

	if (is_ldc_int(*c, &x) && x == 0 && is_if_icmpeq(next(*c), &l))
	{
		return replace(c, 2, makeCODEifeq(l, NULL));
	}

	if (is_ldc_int(*c, &x) && x == 0 && is_if_icmpne(next(*c), &l))
	{
		return replace(c, 2, makeCODEifne(l, NULL));
	}
	return 0;
}

/*
 * dup                  dup
 * if_acmpeq L          if_icmpeq L
 * -------->            -------->
 * goto L               goto L
 */
int simplifyDupEqualityComparison(CODE **c)
{ 
	int l;
  /* if_acmpeq */
  if (is_dup(*c) && is_if_acmpeq(next(*c), &l)) {
	return replace_modified(c, 2, makeCODEgoto(l, NULL));
  }

  /* if_icmpeq */
  if (is_dup(*c) && is_if_icmpeq(next(*c), &l)) {
	return replace_modified(c, 2, makeCODEgoto(l, NULL));
  }

  return 0;
}

/* dead_label:
 * op_1
 * --------->
 * op_1
 */

int removeDeadLabel(CODE **c)
{
	int l;

	if (is_label(*c, &l) && deadlabel(l))
	{
		return replace(c, 1, NULL);
	}
	return 0;
}

/* if_icmpeq L1         if_icmpne L1
 * iconst_0             iconst_0
 * goto L2              goto L2
 * L1:                  L1:
 * iconst_1             iconst_1
 * L2:                  L2:
 * ifeq L3              ifeq L3
 * --------->           --------->
 * if_icmpne L3         if_icmpeq L3
 */

int simplifyEqualBranch(CODE **c)
{
	int l1, l2, l3;
	int x, y;

	if (is_if_icmpeq(*c, &l1) &&
		is_ldc_int(next(*c), &x) &&
		is_goto(next(next(*c)), &l2) &&
		is_ldc_int(next(destination(l1)), &y) &&
		is_ifeq(next(destination(l2)), &l3) &&
		x == 0 && y == 1)
	{
		return replace(c, 7, makeCODEif_icmpne(l3, NULL));
	}
	if (is_if_icmpne(*c, &l1) &&
		is_ldc_int(next(*c), &x) &&
		is_goto(next(next(*c)), &l2) &&
		is_ldc_int(next(destination(l1)), &y) &&
		is_ifeq(next(destination(l2)), &l3) &&
		x == 0 && y == 1)

	{
		return replace(c, 7, makeCODEif_icmpeq(l3, NULL));
	}
	return 0;
}

/* 
 * branch1 L1         
 *
 *	(branch1: ifeq, ifne, ifnull, ifnonnull, if_icmpeq,
 *   ificmpne, if_icmpgt, if_icmplt, if_icmpge, if_icmple,
 *   if_acmpeq, or if_acmpne) (NOT goto)
 *
 *                    (L1: Unique)
 * iconst_0
 * goto L2            (L2: Unique)
 * L1:
 * iconst_1
 * L2:
 * ifeq L3
 * --------->
 * branch2 L3         

 *  (branch2: ifne, ifeq, ifnonnull, ifnull, if_icmpne,
 *   ificmpeq, if_icmple, if_icmpge, if_icmplt, if_icmpgt,
 *   if_acmpne, or if_acmpeq, respectively, depending on the
 *   identity of branch1
 *  )
 */

int collapseLocalBranch(CODE **c) {
	int x1, x2, l1, l2, l3, l4, l5;

	/* 
		ifne 
		--> 
		ifeq 
	*/
	if (is_ifne(*c, &l1) && uniquelabel(l1) &&
	    is_ldc_int(next(*c), &x1) && x1 == 0 &&
	    is_goto(nextby(*c, 2), &l2) && uniquelabel(l2) &&
	    is_label(nextby(*c, 3), &l3) && l3 == l1 &&
	    is_ldc_int(nextby(*c, 4), &x2) && x2 == 1 &&
	    is_label(nextby(*c, 5), &l4) && l4 == l2 &&
	    is_ifeq(nextby(*c, 6), &l5)) {
		return replace(c, 7, makeCODEifeq(l5, NULL));
	}

	/* 
		ifeq 
		--> 
		ifne 
	*/
	if (is_ifeq(*c, &l1) && uniquelabel(l1) &&
	    is_ldc_int(next(*c), &x1) && x1 == 0 &&
	    is_goto(nextby(*c, 2), &l2) && uniquelabel(l2) &&
	    is_label(nextby(*c, 3), &l3) && l3 == l1 &&
	    is_ldc_int(nextby(*c, 4), &x2) && x2 == 1 &&
	    is_label(nextby(*c, 5), &l4) && l4 == l2 &&
	    is_ifeq(nextby(*c, 6), &l5)) {
		return replace(c, 7, makeCODEifne(l5, NULL));
	}

	/* 
		ifnull 
		--> 
		ifnonnull 
	*/
	if (is_ifnull(*c, &l1) && uniquelabel(l1) &&
	    is_ldc_int(next(*c), &x1) && x1 == 0 &&
	    is_goto(nextby(*c, 2), &l2) && uniquelabel(l2) &&
	    is_label(nextby(*c, 3), &l3) && l3 == l1 &&
	    is_ldc_int(nextby(*c, 4), &x2) && x2 == 1 &&
	    is_label(nextby(*c, 5), &l4) && l4 == l2 &&
	    is_ifeq(nextby(*c, 6), &l5)) {
		return replace(c, 7, makeCODEifnonnull(l5, NULL));
	}

	/* 
		if_icmpeq 
		--> 
		if_icmpne 
	*/
	if (is_if_icmpeq(*c, &l1) && uniquelabel(l1) &&
	    is_ldc_int(next(*c), &x1) && x1 == 0 &&
	    is_goto(nextby(*c, 2), &l2) && uniquelabel(l2) &&
	    is_label(nextby(*c, 3), &l3) && l3 == l1 &&
	    is_ldc_int(nextby(*c, 4), &x2) && x2 == 1 &&
	    is_label(nextby(*c, 5), &l4) && l4 == l2 &&
	    is_ifeq(nextby(*c, 6), &l5)) {
		return replace(c, 7, makeCODEif_icmpne(l5, NULL));
	}

	/* 
		if_icmpne 
		--> 
		if_icmpeq 
	*/
	if (is_if_icmpne(*c, &l1) && uniquelabel(l1) &&
	    is_ldc_int(next(*c), &x1) && x1 == 0 &&
	    is_goto(nextby(*c, 2), &l2) && uniquelabel(l2) &&
	    is_label(nextby(*c, 3), &l3) && l3 == l1 &&
	    is_ldc_int(nextby(*c, 4), &x2) && x2 == 1 &&
	    is_label(nextby(*c, 5), &l4) && l4 == l2 &&
	    is_ifeq(nextby(*c, 6), &l5)) {
		return replace(c, 7, makeCODEif_icmpeq(l5, NULL));
	}

	/* 
		if_icmpgt 
		--> 
		if_icmple 
	*/
	if (is_if_icmpgt(*c, &l1) && uniquelabel(l1) &&
	    is_ldc_int(next(*c), &x1) && x1 == 0 &&
	    is_goto(nextby(*c, 2), &l2) && uniquelabel(l2) &&
	    is_label(nextby(*c, 3), &l3) && l3 == l1 &&
	    is_ldc_int(nextby(*c, 4), &x2) && x2 == 1 &&
	    is_label(nextby(*c, 5), &l4) && l4 == l2 &&
	    is_ifeq(nextby(*c, 6), &l5)) {
		return replace(c, 7, makeCODEif_icmple(l5, NULL));
	}

	/* 
		if_icmplt 
		--> 
		if_icmpge 
	*/
	if (is_if_icmplt(*c, &l1) && uniquelabel(l1) &&
	    is_ldc_int(next(*c), &x1) && x1 == 0 &&
	    is_goto(nextby(*c, 2), &l2) && uniquelabel(l2) &&
	    is_label(nextby(*c, 3), &l3) && l3 == l1 &&
	    is_ldc_int(nextby(*c, 4), &x2) && x2 == 1 &&
	    is_label(nextby(*c, 5), &l4) && l4 == l2 &&
	    is_ifeq(nextby(*c, 6), &l5)) {
		return replace(c, 7, makeCODEif_icmpge(l5, NULL));
	}

	/* 
		ifnonnull 
		--> 
		ifnull 
	*/
	if (is_ifnonnull(*c, &l1) && uniquelabel(l1) &&
	    is_ldc_int(next(*c), &x1) && x1 == 0 &&
	    is_goto(nextby(*c, 2), &l2) && uniquelabel(l2) &&
	    is_label(nextby(*c, 3), &l3) && l3 == l1 &&
	    is_ldc_int(nextby(*c, 4), &x2) && x2 == 1 &&
	    is_label(nextby(*c, 5), &l4) && l4 == l2 &&
	    is_ifeq(nextby(*c, 6), &l5)) {
		return replace(c, 7, makeCODEifnull(l5, NULL));
	}

	/* 
		if_icmpge 
		--> 
		if_icmplt 
	*/
	if (is_if_icmpge(*c, &l1) && uniquelabel(l1) &&
	    is_ldc_int(next(*c), &x1) && x1 == 0 &&
	    is_goto(nextby(*c, 2), &l2) && uniquelabel(l2) &&
	    is_label(nextby(*c, 3), &l3) && l3 == l1 &&
	    is_ldc_int(nextby(*c, 4), &x2) && x2 == 1 &&
	    is_label(nextby(*c, 5), &l4) && l4 == l2 &&
	    is_ifeq(nextby(*c, 6), &l5)) {
		return replace(c, 7, makeCODEif_icmplt(l5, NULL));
	}

	/* 
		if_acmpeq 
		--> 
		if_acmpne 
	*/
	if (is_if_acmpeq(*c, &l1) && uniquelabel(l1) &&
	    is_ldc_int(next(*c), &x1) && x1 == 0 &&
	    is_goto(nextby(*c, 2), &l2) && uniquelabel(l2) &&
	    is_label(nextby(*c, 3), &l3) && l3 == l1 &&
	    is_ldc_int(nextby(*c, 4), &x2) && x2 == 1 &&
	    is_label(nextby(*c, 5), &l4) && l4 == l2 &&
	    is_ifeq(nextby(*c, 6), &l5)) {
		return replace(c, 7, makeCODEif_acmpne(l5, NULL));
	}

	/* 
		if_acmpne 
		--> 
		if_acmpeq 
	*/
	if (is_if_acmpne(*c, &l1) && uniquelabel(l1) &&
	    is_ldc_int(next(*c), &x1) && x1 == 0 &&
	    is_goto(nextby(*c, 2), &l2) && uniquelabel(l2) &&
	    is_label(nextby(*c, 3), &l3) && l3 == l1 &&
	    is_ldc_int(nextby(*c, 4), &x2) && x2 == 1 &&
	    is_label(nextby(*c, 5), &l4) && l4 == l2 &&
	    is_ifeq(nextby(*c, 6), &l5)) {
		return replace(c, 7, makeCODEif_acmpeq(l5, NULL));
	}

	/* 
		if_icmple 
		--> 
		if_icmpgt 
	*/
	if (is_if_icmple(*c, &l1) && uniquelabel(l1) &&
	    is_ldc_int(next(*c), &x1) && x1 == 0 &&
	    is_goto(nextby(*c, 2), &l2) && uniquelabel(l2) &&
	    is_label(nextby(*c, 3), &l3) && l3 == l1 &&
	    is_ldc_int(nextby(*c, 4), &x2) && x2 == 1 &&
	    is_label(nextby(*c, 5), &l4) && l4 == l2 &&
	    is_ifeq(nextby(*c, 6), &l5)) {
		return replace(c, 7, makeCODEif_icmpgt(l5, NULL));
	}

	return 0;
}

/* essentially flip comparisons on branch goto, must be unique label to apply */
/*
 * ifnull L1  	 ifnonnull L1 	ifeq L1		ifne L1    if_icmpeq L1 	if_icmpne L1 	 	
 * goto L2 		
 * L1:			
 * --------->	
 * ifnonnull L2  ifnull L2  	ifne L2 	ifeq L2 	if_icmpne L2 	if_icmpeq L2
 */

/*
 * if_icmplt L1 	if_icmpgt  L1	if_icmple L1    if_icmpge L1 	if_acmpeq L1 	if_acmpne L1 	
 * goto L2 		
 * L1:			
 * --------->	
 * if_icmpge L2   	if_icmple L2 	 if_icmpgt L2 	if_icmplt L2 	if_acmpne L2 	if_acmpeq L2
 */

int simplifyBranchGoto(CODE **c)
{ 
	int l1, l2, l3;

	if (is_ifnull(*c, &l1) && 
  		uniquelabel(l1) &&
		is_goto(next(*c), &l2) &&
      	is_label(nextby(*c, 2), &l3) && 
      	l3 == l1)
    	return replace(c, 3, makeCODEifnonnull(l2, NULL));

  	if (is_ifnonnull(*c, &l1) && 
  		uniquelabel(l1) &&
  		is_goto(next(*c), &l2) &&
  		is_label(nextby(*c, 2), &l3) && 
  		l3 == l1) 
		return replace(c, 3, makeCODEifnull(l2, NULL));

	if (is_ifeq(*c, &l1) && 
		uniquelabel(l1) &&
	    is_goto(next(*c), &l2) &&
	    is_label(nextby(*c, 2), &l3) && 
	    l3 == l1) 
	    return replace(c, 3, makeCODEifne(l2, NULL));	

    if (is_ifne(*c, &l1) &&
    	uniquelabel(l1) &&
      	is_goto(next(*c), &l2) &&
      	is_label(nextby(*c, 2), &l3) && 
      	l3 == l1) 
		return replace(c, 3, makeCODEifeq(l2, NULL));

  	if (is_if_icmpeq(*c, &l1) && 
  		uniquelabel(l1) &&
  		is_goto(next(*c), &l2) &&
  		is_label(nextby(*c, 2), &l3) && 
  		l3 == l1) 
  		return replace(c, 3, makeCODEif_icmpne(l2, NULL));

  	if (is_if_icmpne(*c, &l1) && 
  		uniquelabel(l1) &&
  		is_goto(next(*c), &l2) &&
  		is_label(nextby(*c, 2), &l3) && l3 == l1)
		return replace(c, 3, makeCODEif_icmpeq(l2, NULL));

  	if (is_if_icmplt(*c, &l1) && 
  		uniquelabel(l1) &&
  		is_goto(next(*c), &l2) &&
  		is_label(nextby(*c, 2), &l3) && 
  		l3 == l1) 
  		return replace(c, 3, makeCODEif_icmpge(l2, NULL));

	if (is_if_icmpgt(*c, &l1) && 
		uniquelabel(l1) &&
		is_goto(next(*c), &l2) &&
		is_label(nextby(*c, 2), &l3) && 
		l3 == l1) 
		return replace(c, 3, makeCODEif_icmple(l2, NULL));

	if (is_if_icmple(*c, &l1) && 
  		uniquelabel(l1) &&
  		is_goto(next(*c), &l2) && 
  		is_label(nextby(*c, 2), &l3) && 
  		l3 == l1)
		return replace(c, 3, makeCODEif_icmpgt(l2, NULL));

  	if (is_if_icmpge(*c, &l1) && 
  		uniquelabel(l1) && 
  		is_goto(next(*c), &l2) &&
  		is_label(nextby(*c, 2), &l3) && 
  		l3 == l1)
    	return replace(c, 3, makeCODEif_icmplt(l2, NULL));

	if (is_if_acmpeq(*c, &l1) && 
		uniquelabel(l1) &&
		is_goto(next(*c), &l2) &&
		is_label(nextby(*c, 2), &l3) && 
		l3 == l1) 
		return replace(c, 3, makeCODEif_acmpne(l2, NULL));

  	if (is_if_acmpne(*c, &l1) && 
  		uniquelabel(l1) &&
  		is_goto(next(*c), &l2) &&
  		is_label(nextby(*c, 2), &l3) && 
  		l3 == l1)
  		return replace(c, 3, makeCODEif_acmpeq(l2, NULL));

  return 0;
}

void init_patterns(void)
{
	ADD_PATTERN(simplify_multiplication_right);
	ADD_PATTERN(simplifyMultiplicationLeft);
	ADD_PATTERN(simplifyAddition);
	ADD_PATTERN(simplifySubtraction);
	ADD_PATTERN(simplifyDivision);

	ADD_PATTERN(simplifyStore);
	ADD_PATTERN(simplifyStoreLoad);

	ADD_PATTERN(positive_increment);
	ADD_PATTERN(negative_increment);

	ADD_PATTERN(simplify_goto_goto);
	ADD_PATTERN(simplifyBranchGoto);
	ADD_PATTERN(removeDeadLabel); 

	ADD_PATTERN(simplifyEqualityComparison);
	ADD_PATTERN(simplifyDupEqualityComparison);

	ADD_PATTERN(simplifyEqualBranch);

	ADD_PATTERN(collapseLocalBranch);
}
