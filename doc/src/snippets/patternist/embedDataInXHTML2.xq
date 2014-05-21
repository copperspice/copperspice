declare namespace x = "http://www.w3.org/1998/xhtml";
<x:html>
    <x:body>
        {
            for $i in doc("testResult.xml")/tests/test[@status = "failure"]
            order by $i/@name
            return <x:p>{$i/@name}</x:p>
        }
    </x:body>
</x:html>
