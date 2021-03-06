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
<xsl:template match="rdf:RDF">
<xsl:for-each select="rdfs:Class">

<xsl:variable name="about" select="@rdf:about"/>

public class <xsl:value-of select="rdfs:label" />
<xsl:if test="count(rdfs:subClassOf) > 0">: <xsl:for-each select="rdfs:subClassOf/rdfs:Class"><xsl:if test="position() > 1">, </xsl:if><xsl:value-of select="substring-after(@rdf:about, '#')"/> </xsl:for-each> 
</xsl:if> {
		
	<xsl:for-each select="/rdf:RDF/rdf:Property/rdfs:domain[@rdf:resource=$about]">

	public <xsl:value-of select="substring-after(../rdfs:range/@rdf:resource, '#')"/><xsl:text> </xsl:text><xsl:value-of select="../rdfs:label"/> {
	}
	
	</xsl:for-each>

}

</xsl:for-each>
</xsl:template>
</xsl:stylesheet> 
