<result>
    <para>The following skin care products have shipped, ordered by shipping date(oldest first):</para>
    {
        for $i in doc("myOrders.xml")/orders/order[@product = "Acme Skin Care"]
        order by xs:date($i/@shippingDate) descending
        return $i
    }
</result>
