using System;//  w w w.  ja va 2s. c om

public class Example
{
   public static void Main()
   {
      Guid guid = Guid.NewGuid();
      Console.WriteLine("Guid: {0}", guid.ToString("N"));
      Console.WriteLine("Guid: {0}", guid.ToString("D"));
      Console.WriteLine("Guid: {0}", guid.ToString("B"));
      Console.WriteLine("Guid: {0}", guid.ToString("P"));
      Console.WriteLine("Guid: {0}", guid.ToString("X"));
   }
}