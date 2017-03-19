<?xml version="1.0"?>

<!--
	Copyright 2014-2017 Katherine Flavel

	See LICENCE for the full copyright terms.
-->

<xsl:stylesheet version="1.0" 
	xmlns="http://www.w3.org/2000/svg"
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
	xmlns:svg="http://www.w3.org/2000/svg"
	xmlns:xlink="http://www.w3.org/1999/xlink">

	<!-- for wrangling asciidia(1) svg output -->

	<xsl:template match="svg:svg">
<xsl:processing-instruction name="xml-stylesheet">href="file:///home/kate/svn/kgt1/trunk/css/asciidia.css" type="text/css"</xsl:processing-instruction>

		<xsl:copy>
			<xsl:apply-templates select="@*"/>

			<xsl:apply-templates select="node()"/>
		</xsl:copy>
	</xsl:template>

	<xsl:template match="svg:*">
		<xsl:copy>
			<xsl:apply-templates select="@*|node()"/>
		</xsl:copy>
	</xsl:template>

	<xsl:template match="@*">
		<xsl:copy/>
	</xsl:template>

	<xsl:template match="svg:path[contains(@style, 'fill: black')]">
		<xsl:copy>
			<xsl:attribute name="class">
				<xsl:text>arrow</xsl:text>
			</xsl:attribute>

			<xsl:apply-templates select="@*|node()"/>
		</xsl:copy>
	</xsl:template>

	<xsl:template match="svg:rect[@rx]">
		<xsl:copy>
			<xsl:attribute name="rx">
				<xsl:text>7</xsl:text>
			</xsl:attribute>

			<xsl:attribute name="ry">
				<xsl:text>10</xsl:text>
			</xsl:attribute>

			<xsl:apply-templates select="@*|node()"/>
		</xsl:copy>
	</xsl:template>

	<xsl:template match="svg:rect[not(@rx)]">
		<xsl:copy>
			<xsl:attribute name="class">
				<xsl:text>terminal</xsl:text>
			</xsl:attribute>

			<xsl:apply-templates select="@*|node()"/>
		</xsl:copy>
	</xsl:template>

	<xsl:template match="@style"/>
	<xsl:template match="@font-family"/>
	<xsl:template match="@font-size"/>
	<xsl:template match="@rx"/>
	<xsl:template match="@ry"/>

</xsl:stylesheet>

