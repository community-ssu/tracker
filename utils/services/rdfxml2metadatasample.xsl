<xsl:stylesheet version = '1.0'
     xmlns:xsl='http://www.w3.org/1999/XSL/Transform'
  xmlns:nid3="http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#"
  xmlns:nfo="http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#"
  xmlns:nmo="http://www.semanticdesktop.org/ontologies/2007/03/22/nmo#"
  xmlns:nie="http://www.semanticdesktop.org/ontologies/2007/01/19/nie#"
  xmlns:exif="http://www.kanzaki.com/ns/exif#"
  xmlns:nao="http://www.semanticdesktop.org/ontologies/2007/08/15/nao#"
  xmlns:rdfs="http://www.w3.org/2000/01/rdf-schema#"
  xmlns:protege="http://protege.stanford.edu/system#"
  xmlns:dcterms="http://purl.org/dc/terms/"
  xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"
  xmlns:ncal="http://www.semanticdesktop.org/ontologies/2007/04/02/ncal#"
  xmlns:xsd="http://www.w3.org/2001/XMLSchema#"
  xmlns:nrl="http://www.semanticdesktop.org/ontologies/2007/08/15/nrl#"
  xmlns:pimo="http://www.semanticdesktop.org/ontologies/2007/11/01/pimo#"
  xmlns:geo="http://www.w3.org/2003/01/geo/wgs84_pos#"
  xmlns:tmo="http://www.semanticdesktop.org/ontologies/2008/05/20/tmo#"
  xmlns:dc="http://purl.org/dc/elements/1.1/"
  xmlns:nco="http://www.semanticdesktop.org/ontologies/2007/03/22/nco#"
  xmlns:nexif="http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#">

<xsl:output method="text" />

<xsl:template name="predicate-of">
 <xsl:param name="about"/>
 <xsl:choose>
  <xsl:when test="substring-after($about, '/')">
   <xsl:call-template name="predicate-of">
     <xsl:with-param name="about" select="substring-after($about, '/')"/>
   </xsl:call-template>
  </xsl:when>
  <xsl:otherwise>
     <xsl:choose>
       <xsl:when test="substring-after($about, '#')">
         <xsl:if test="substring-before($about, '#') != 'XMLSchema'"><xsl:value-of select="substring-before($about, '#')"/>:</xsl:if><xsl:value-of select="substring-after($about, '#')"/>
       </xsl:when>
       <xsl:otherwise>DC:<xsl:value-of select="$about"/></xsl:otherwise>
    </xsl:choose>
  </xsl:otherwise>
 </xsl:choose>
</xsl:template>

<xsl:template match="rdf:RDF">
<xsl:for-each select="rdf:Property">

[<xsl:call-template name="predicate-of"><xsl:with-param name="about"><xsl:value-of select="@rdf:about"/></xsl:with-param></xsl:call-template>]
<xsl:choose>
<xsl:when test="rdfs:range">
DataType=<xsl:call-template name="predicate-of"><xsl:with-param name="about"><xsl:value-of select="rdfs:range/@rdf:resource"/></xsl:with-param></xsl:call-template>
</xsl:when>
<xsl:otherwise>Abstract=true
DataType=string</xsl:otherwise>
</xsl:choose>
<xsl:if test="rdfs:label">
DisplayName=<xsl:value-of select="rdfs:label"/>
</xsl:if>

<xsl:if test="rdfs:domain">
Domain=<xsl:call-template name="predicate-of"><xsl:with-param name="about"><xsl:value-of select="rdfs:domain/@rdf:resource"/></xsl:with-param></xsl:call-template></xsl:if>

<xsl:if test="rdfs:subPropertyOf">
SuperProperties=<xsl:for-each select="rdfs:subPropertyOf"><xsl:if test="@rdf:resource"><xsl:call-template name="predicate-of"><xsl:with-param name="about"><xsl:value-of select="@rdf:resource"/></xsl:with-param></xsl:call-template>;</xsl:if><xsl:for-each select="rdfs:subPropertyOf/rdf:Property"><xsl:call-template name="predicate-of"><xsl:with-param name="about"><xsl:value-of select="@rdf:about"/></xsl:with-param></xsl:call-template>;</xsl:for-each></xsl:for-each></xsl:if>

<xsl:if test="rdfs:comment">
Description=<xsl:value-of select="rdfs:comment"/></xsl:if>
</xsl:for-each>
</xsl:template>
</xsl:stylesheet> 
