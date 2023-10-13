// Fill out your copyright notice in the Description page of Project Settings.


#include "ThrowableWidget.h"

void UThrowableWidget::UpdateThrowableCount(int32 NewCount, int32 NewTotalCount)
{
	Count = NewCount;
	TotalCount = NewTotalCount;
}
