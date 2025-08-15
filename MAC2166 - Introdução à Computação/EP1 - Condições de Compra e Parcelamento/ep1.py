"""
  AO PREENCHER ESSE CABEÇALHO COM O MEU NOME E O MEU NÚMERO USP,
  DECLARO QUE SOU O ÚNICO AUTOR E RESPONSÁVEL POR ESSE PROGRAMA.
  TODAS AS PARTES ORIGINAIS DESSE EXERCÍCIO PROGRAMA (EP) FORAM
  DESENVOLVIDAS E IMPLEMENTADAS POR MIM SEGUINDO AS INSTRUÇÕES
  DESSE EP E QUE PORTANTO NÃO CONSTITUEM DESONESTIDADE ACADÊMICA
  OU PLÁGIO.
  DECLARO TAMBÉM QUE SOU RESPONSÁVEL POR TODAS AS CÓPIAS
  DESSE PROGRAMA E QUE EU NÃO DISTRIBUI OU FACILITEI A
  SUA DISTRIBUIÇÃO. ESTOU CIENTE QUE OS CASOS DE PLÁGIO E
  DESONESTIDADE ACADÊMICA SERÃO TRATADOS SEGUNDO OS CRITÉRIOS
  DIVULGADOS NA PÁGINA DA DISCIPLINA.
  ENTENDO QUE EPS SEM ASSINATURA NÃO SERÃO CORRIGIDOS E,
  AINDA ASSIM, PODERÃO SER PUNIDOS POR DESONESTIDADE ACADÊMICA.

  Nome : Antônio Augusto Carnevalli
  NUSP : 13682909
  Turma: Engenharia Mecatrônica - 11
  Prof.: Alair Pereira do Lago

  Referências: Com exceção das rotinas fornecidas no enunciado
  e em sala de aula, caso você tenha utilizado alguma refência,
  liste-as abaixo para que o seu programa não seja considerado
  plágio ou irregular.

  Exemplo:
  - O algoritmo Quicksort foi baseado em
  http://wiki.python.org.br/QuickSort

  """
#MAC-2166 EP 01 - XIMBICA GLX
print("Modos de operacao:")
print("1 - Valor presente de um financiamento")
print("2 - Valor minimo da prestacao para valor presente cobrir pagamento a vista")
print("3 - Comparacao de opcoes de pagamento")
print("4 - Tabela comparativa de opcoes de pagamento para taxas variadas")
modo = int(input("Numero do modo desejado: "))
if modo == 1:
    p = int(input("Valor da prestacao: "))
    i = int(input("Taxa de juros: "))
    n = int(input("Numero de prestacoes: "))
    VP = int(p/(i/1000))*(1+(i/1000)-(1/((1+(i/1000))**(n-1))))
    print("Valor presente:", int(VP))    
elif modo == 2:       
    valoravista = int(input("Valor a vista: "))
    i = int(input("Taxa de juros: "))
    n = int(input("Numero de prestacoes: "))
    prestacao = ((valoravista) * (i / 1000))  / (1 + (i / 1000) - (1 / (1 + (i / 1000)) ** (n - 1))) + 1
    if prestacao==int(prestacao):        
        print("Menor prestacao cujo valor presente do financiamento cobre o valor a vista:", int(prestacao-1))
    else:
        print("Menor prestacao cujo valor presente do financiamento cobre o valor a vista:", int(prestacao))   
elif modo == 3:
    valoravista = int(input("Valor a vista: "))
    i = int(input("Taxa de juros: "))
    vpre12 = int(input("Valor da prestacao em 12 vezes: "))
    vpre24 = int(input("Valor da prestacao em 24 vezes: "))
    VP12 = ((vpre12) / (i / 1000)) * (1 + (i / 1000) - (1 / ((1 + (i / 1000)) ** (11))))
    VP24 = ((vpre24) / (i / 1000)) * (1 + (i / 1000) - (1 / ((1 + (i / 1000)) ** (23))))
    print("Valor presente em 12 vezes:", int(VP12))
    print("Valor presente em 24 vezes:", int(VP24))
    VP12=int(VP12)
    VP24=int(VP24)
    if VP24 < VP12 and VP24 < valoravista:
        print("Numero de parcelas da melhor opcao de pagamento: 24")
    elif VP12 <= VP24 and VP12 < valoravista:
        print("Numero de parcelas da melhor opcao de pagamento: 12")
    else:
        print("Numero de parcelas da melhor opcao de pagamento: 1")                
elif modo == 4:
    valoravista = int(input("Valor a vista: "))
    vpre12 = int(input("Valor da prestacao em 12 vezes: "))
    vpre24 = int(input("Valor da prestacao em 24 vezes: "))
    tax = int(input("Inicio da sequencia das taxas: "))
    ntax = int(input("Numero de taxas: "))
    inctax = int(input("Incremento de uma taxa para a proxima: "))
    contador = int(0)
    print("taxa |  a vista | 12 vezes | 24 vezes | decisao")
    while contador < ntax:
        VP12 = (vpre12 / (tax / 1000)) * (1 + (tax / 1000) - (1 / ((1 + (tax / 1000)) ** (11))))
        VP24 = (vpre24 / (tax / 1000)) * (1 + (tax / 1000) - (1 / ((1 + (tax / 1000)) ** (23))))
        if VP24 < VP12 and VP24 < valoravista:
            print("%4.1i" %tax, "|","%8.2i" %valoravista, "|","%8.2i" %VP12, "|","%8.2i" %VP24, "|", int(24))
            contador+=1
            tax = tax + inctax
        elif VP12 <= VP24 and VP12 < valoravista:
            print("%4.1i" %tax, "|","%8.2i" %valoravista, "|","%8.2i" %VP12, "|","%8.2i" %VP24, "|", int(12))
            contador+=1
            tax = tax + inctax
        else:
            print("%4.1i" %tax, "|","%8.2i" %valoravista, "|","%8.2i" %VP12, "|","%8.2i" %VP24, "|", int(1))
            contador+=1
            tax = tax + inctax