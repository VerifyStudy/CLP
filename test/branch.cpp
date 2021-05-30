void foo(int a)
{
  // have part then but not block
  // no part else
  if (a > 1)
    a = 1;
  // have part else but not block
  if (a>2)
    a = 2;
  else
    a = 2;
  // have part else and block
  if (a>6)
    a = 6;
  else
  {
    a = 6;
  }

  // have part then and block
  // no part else
  if (a > 3)
  {
    a = 3;
  }
  // have part else but no block
  if (a > 4)
  {
    a = 4;
  }
  else
    a = 4;

  // have part else and block
  if (a > 5)
  {
    a = 5;
  }
  else
  {
    a = 5;
  }
  int abc;
  //switch->if
  switch (abc)
  {
    case '1': 
      a=1;
      break;
    case '2':
      a=2;
    case '4': 
      a=4;
      break;
    default:
      a=3;
    }

}