// Copyright 2017 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/*!
 * @file Payloads.cpp
 * This source file contains the definition of the described types in the IDL file.
 *
 * This file was generated by the tool gen.
 */

#ifdef _WIN32
// Remove linker warning LNK4221 on Visual Studio
namespace {
char dummy;
}
#endif

#include "Payloads.h"

#include <fastcdr/Cdr.h>

#include <fastcdr/exceptions/BadParamException.h>
using namespace eprosima::fastcdr::exception;

#include <utility>

eprosima::micrortps::CREATE_Payload::CREATE_Payload()
{
}

eprosima::micrortps::CREATE_Payload::~CREATE_Payload()
{
}

eprosima::micrortps::CREATE_Payload::CREATE_Payload(const CREATE_Payload &x) : BaseObjectRequest(x)
{
    m_object_representation = x.m_object_representation;
}

eprosima::micrortps::CREATE_Payload::CREATE_Payload(CREATE_Payload &&x) : BaseObjectRequest(x)
{
    m_object_representation = std::move(x.m_object_representation);
}

eprosima::micrortps::CREATE_Payload &eprosima::micrortps::CREATE_Payload::operator=(const CREATE_Payload &x)
{
    BaseObjectRequest::operator=(x);
    m_object_representation    = x.m_object_representation;

    return *this;
}

eprosima::micrortps::CREATE_Payload &eprosima::micrortps::CREATE_Payload::operator=(CREATE_Payload &&x)
{
    BaseObjectRequest::operator=(x);
    m_object_representation    = std::move(x.m_object_representation);

    return *this;
}

size_t eprosima::micrortps::CREATE_Payload::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += eprosima::micrortps::BaseObjectRequest::getMaxCdrSerializedSize(current_alignment);
    current_alignment += eprosima::micrortps::ObjectVariant::getMaxCdrSerializedSize(current_alignment);

    return current_alignment - initial_alignment;
}

size_t eprosima::micrortps::CREATE_Payload::getCdrSerializedSize(const eprosima::micrortps::CREATE_Payload &data,
                                                                 size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += eprosima::micrortps::BaseObjectRequest::getCdrSerializedSize(data, current_alignment);
    current_alignment +=
        eprosima::micrortps::ObjectVariant::getCdrSerializedSize(data.object_representation(), current_alignment);

    return current_alignment - initial_alignment;
}

void eprosima::micrortps::CREATE_Payload::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    BaseObjectRequest::serialize(scdr);
    scdr << m_object_representation;
}

void eprosima::micrortps::CREATE_Payload::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    BaseObjectRequest::deserialize(dcdr);
    dcdr >> m_object_representation;
}

eprosima::micrortps::DELETE_RESOURCE_Payload::DELETE_RESOURCE_Payload()
{
}

eprosima::micrortps::DELETE_RESOURCE_Payload::~DELETE_RESOURCE_Payload()
{
}

eprosima::micrortps::DELETE_RESOURCE_Payload::DELETE_RESOURCE_Payload(const DELETE_RESOURCE_Payload &x)
    : BaseObjectRequest(x)
{
}

eprosima::micrortps::DELETE_RESOURCE_Payload::DELETE_RESOURCE_Payload(DELETE_RESOURCE_Payload &&x)
    : BaseObjectRequest(x)
{
}

eprosima::micrortps::DELETE_RESOURCE_Payload &eprosima::micrortps::DELETE_RESOURCE_Payload::
operator=(const DELETE_RESOURCE_Payload &x)
{
    BaseObjectRequest::operator=(x);

    return *this;
}

eprosima::micrortps::DELETE_RESOURCE_Payload &eprosima::micrortps::DELETE_RESOURCE_Payload::
operator=(DELETE_RESOURCE_Payload &&x)
{
    BaseObjectRequest::operator=(x);

    return *this;
}

size_t eprosima::micrortps::DELETE_RESOURCE_Payload::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += eprosima::micrortps::BaseObjectRequest::getMaxCdrSerializedSize(current_alignment);

    return current_alignment - initial_alignment;
}

size_t eprosima::micrortps::DELETE_RESOURCE_Payload::getCdrSerializedSize(
    const eprosima::micrortps::DELETE_RESOURCE_Payload &data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += eprosima::micrortps::BaseObjectRequest::getCdrSerializedSize(data, current_alignment);

    return current_alignment - initial_alignment;
}

void eprosima::micrortps::DELETE_RESOURCE_Payload::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    BaseObjectRequest::serialize(scdr);
}

void eprosima::micrortps::DELETE_RESOURCE_Payload::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    BaseObjectRequest::deserialize(dcdr);
}

eprosima::micrortps::RESOURCE_STATUS_Payload::RESOURCE_STATUS_Payload()
{
}

eprosima::micrortps::RESOURCE_STATUS_Payload::~RESOURCE_STATUS_Payload()
{
}

eprosima::micrortps::RESOURCE_STATUS_Payload::RESOURCE_STATUS_Payload(const RESOURCE_STATUS_Payload &x)
    : BaseObjectReply(x)
{
}

eprosima::micrortps::RESOURCE_STATUS_Payload::RESOURCE_STATUS_Payload(RESOURCE_STATUS_Payload &&x) : BaseObjectReply(x)
{
}

eprosima::micrortps::RESOURCE_STATUS_Payload &eprosima::micrortps::RESOURCE_STATUS_Payload::
operator=(const RESOURCE_STATUS_Payload &x)
{
    BaseObjectReply::operator=(x);
    return *this;
}

eprosima::micrortps::RESOURCE_STATUS_Payload &eprosima::micrortps::RESOURCE_STATUS_Payload::
operator=(RESOURCE_STATUS_Payload &&x)
{
    BaseObjectReply::operator=(x);
    return *this;
}

size_t eprosima::micrortps::RESOURCE_STATUS_Payload::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += eprosima::micrortps::BaseObjectReply::getMaxCdrSerializedSize(current_alignment);

    return current_alignment - initial_alignment;
}

size_t eprosima::micrortps::RESOURCE_STATUS_Payload::getCdrSerializedSize(
    const eprosima::micrortps::RESOURCE_STATUS_Payload &data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += eprosima::micrortps::BaseObjectReply::getCdrSerializedSize(data, current_alignment);

    return current_alignment - initial_alignment;
}

void eprosima::micrortps::RESOURCE_STATUS_Payload::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    BaseObjectReply::serialize(scdr);
}

void eprosima::micrortps::RESOURCE_STATUS_Payload::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    BaseObjectReply::deserialize(dcdr);
}

eprosima::micrortps::GET_INFO_Payload::GET_INFO_Payload()
{

    m_info_mask = eprosima::micrortps::INFO_CONFIGURATION;
}

eprosima::micrortps::GET_INFO_Payload::~GET_INFO_Payload()
{
}

eprosima::micrortps::GET_INFO_Payload::GET_INFO_Payload(const GET_INFO_Payload &x) : BaseObjectRequest(x);
{
    m_info_mask = x.m_info_mask;
}

eprosima::micrortps::GET_INFO_Payload::GET_INFO_Payload(GET_INFO_Payload &&x) : BaseObjectRequest(x);
{
    m_info_mask = x.m_info_mask;
}

eprosima::micrortps::GET_INFO_Payload &eprosima::micrortps::GET_INFO_Payload::operator=(const GET_INFO_Payload &x)
{
    BaseObjectRequest::operator=(x);
    m_info_mask                = x.m_info_mask;

    return *this;
}

eprosima::micrortps::GET_INFO_Payload &eprosima::micrortps::GET_INFO_Payload::operator=(GET_INFO_Payload &&x)
{
    BaseObjectRequest::operator=(x);
    m_info_mask                = x.m_info_mask;

    return *this;
}

size_t eprosima::micrortps::GET_INFO_Payload::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += eprosima::micrortps::BaseObjectRequest::getMaxCdrSerializedSize(current_alignment);
    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);

    return current_alignment - initial_alignment;
}

size_t eprosima::micrortps::GET_INFO_Payload::getCdrSerializedSize(const eprosima::micrortps::GET_INFO_Payload &data,
                                                                   size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += eprosima::micrortps::BaseObjectRequest::getCdrSerializedSize(data, current_alignment);
    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);

    return current_alignment - initial_alignment;
}

void eprosima::micrortps::GET_INFO_Payload::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    BaseObjectRequest::serialize(scdr);
    scdr << (uint32_t)m_info_mask;
}

void eprosima::micrortps::GET_INFO_Payload::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    BaseObjectRequest::deserialize(dcdr);
    dcdr >> (uint32_t &)m_info_mask;
}

eprosima::micrortps::INFO_Payload::INFO_Payload()
{
}

eprosima::micrortps::INFO_Payload::~INFO_Payload()
{
}

eprosima::micrortps::INFO_Payload::INFO_Payload(const INFO_Payload &x) : BaseObjectReply(x)
{
    m_info = x.m_info;
}

eprosima::micrortps::INFO_Payload::INFO_Payload(INFO_Payload &&x) : BaseObjectReply(x)
{
    m_info = std::move(x.m_info);
}

eprosima::micrortps::INFO_Payload &eprosima::micrortps::INFO_Payload::operator=(const INFO_Payload &x)
{
    BaseObjectReply::operator=(x);
    m_info                   = x.m_info;
    return *this;
}

eprosima::micrortps::INFO_Payload &eprosima::micrortps::INFO_Payload::operator=(INFO_Payload &&x)
{
    BaseObjectReply::operator=(x);
    m_info                   = std::move(x.m_info);

    return *this;
}

size_t eprosima::micrortps::INFO_Payload::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += eprosima::micrortps::BaseObjectReply::getMaxCdrSerializedSize(current_alignment);
    current_alignment += eprosima::micrortps::Info::getMaxCdrSerializedSize(current_alignment);

    return current_alignment - initial_alignment;
}

size_t eprosima::micrortps::INFO_Payload::getCdrSerializedSize(const eprosima::micrortps::INFO_Payload &data,
                                                               size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += eprosima::micrortps::BaseObjectReply::getCdrSerializedSize(data, current_alignment);
    current_alignment += eprosima::micrortps::Info::getCdrSerializedSize(data.info(), current_alignment);

    return current_alignment - initial_alignment;
}

void eprosima::micrortps::INFO_Payload::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    BaseObjectReply::serialize(scdr);
    scdr << m_info;
}

void eprosima::micrortps::INFO_Payload::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    BaseObjectReply::deserialize(dcdr);
    dcdr >> m_info;
}

eprosima::micrortps::READ_DATA_Payload::READ_DATA_Payload()
{
}

eprosima::micrortps::READ_DATA_Payload::~READ_DATA_Payload()
{
}

eprosima::micrortps::READ_DATA_Payload::READ_DATA_Payload(const READ_DATA_Payload &x) : BaseObjectRequest(x)
{
    m_read_specification = x.m_read_specification;
}

eprosima::micrortps::READ_DATA_Payload::READ_DATA_Payload(READ_DATA_Payload &&x) : BaseObjectRequest(x);
{
    m_read_specification = std::move(x.m_read_specification);
}

eprosima::micrortps::READ_DATA_Payload &eprosima::micrortps::READ_DATA_Payload::operator=(const READ_DATA_Payload &x)
{
    BaseObjectRequest::operator=(x);
    m_read_specification       = x.m_read_specification;

    return *this;
}

eprosima::micrortps::READ_DATA_Payload &eprosima::micrortps::READ_DATA_Payload::operator=(READ_DATA_Payload &&x)
{
    BaseObjectRequest::operator=(x);
    m_read_specification       = std::move(x.m_read_specification);

    return *this;
}

size_t eprosima::micrortps::READ_DATA_Payload::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += eprosima::micrortps::BaseObjectRequest::getMaxCdrSerializedSize(current_alignment);
    current_alignment += eprosima::micrortps::ReadSpecification::getMaxCdrSerializedSize(current_alignment);

    return current_alignment - initial_alignment;
}

size_t eprosima::micrortps::READ_DATA_Payload::getCdrSerializedSize(const eprosima::micrortps::READ_DATA_Payload &data,
                                                                    size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += eprosima::micrortps::BaseObjectRequest::getCdrSerializedSize(data, current_alignment);
    current_alignment +=
        eprosima::micrortps::ReadSpecification::getCdrSerializedSize(data.read_specification(), current_alignment);

    return current_alignment - initial_alignment;
}

void eprosima::micrortps::READ_DATA_Payload::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    BaseObjectRequest::serialize(scdr);
    scdr << m_read_specification;
}

void eprosima::micrortps::READ_DATA_Payload::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    BaseObjectRequest.deserialize(dcdr);
    dcdr >> m_read_specification;
}

eprosima::micrortps::DATA_Payload_Data::DATA_Payload_Data()
{
}

eprosima::micrortps::DATA_Payload_Data::~DATA_Payload_Data()
{
}

eprosima::micrortps::DATA_Payload_Data::DATA_Payload_Data(const DATA_Payload_Data &x) : BaseObjectReply(x)
{
    m_data = x.m_data;
}

eprosima::micrortps::DATA_Payload_Data::DATA_Payload_Data(DATA_Payload_Data &&x) : BaseObjectReply(x)
{
    m_data = std::move(x.m_data);
}

eprosima::micrortps::DATA_Payload_Data &eprosima::micrortps::DATA_Payload_Data::operator=(const DATA_Payload_Data &x)
{
    BaseObjectReply::operator=(x);
    m_data                   = x.m_data;

    return *this;
}

eprosima::micrortps::DATA_Payload_Data &eprosima::micrortps::DATA_Payload_Data::operator=(DATA_Payload_Data &&x)
{
    BaseObjectReply::operator=(x);
    m_data                   = std::move(x.m_data);

    return *this;
}

size_t eprosima::micrortps::DATA_Payload_Data::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += eprosima::micrortps::BaseObjectReply::getMaxCdrSerializedSize(current_alignment);
    current_alignment += eprosima::micrortps::SampleData::getMaxCdrSerializedSize(current_alignment);

    return current_alignment - initial_alignment;
}

size_t eprosima::micrortps::DATA_Payload_Data::getCdrSerializedSize(const eprosima::micrortps::DATA_Payload_Data &data,
                                                                    size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += eprosima::micrortps::BaseObjectReply::getCdrSerializedSize(data, current_alignment);
    current_alignment += eprosima::micrortps::SampleData::getCdrSerializedSize(data.data(), current_alignment);

    return current_alignment - initial_alignment;
}

void eprosima::micrortps::DATA_Payload_Data::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    BaseObjectReply::serialize(scdr);
    scdr << m_data;
}

void eprosima::micrortps::DATA_Payload_Data::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    BaseObjectReply::deserialize(scdr);
    dcdr >> m_data;
}

eprosima::micrortps::DATA_Payload_Sample::DATA_Payload_Sample()
{
}

eprosima::micrortps::DATA_Payload_Sample::~DATA_Payload_Sample()
{
}

eprosima::micrortps::DATA_Payload_Sample::DATA_Payload_Sample(const DATA_Payload_Sample &x) : BaseObjectReply(x)
{
    m_sample = x.m_sample;
}

eprosima::micrortps::DATA_Payload_Sample::DATA_Payload_Sample(DATA_Payload_Sample &&x) : BaseObjectReply(x)
{
    m_sample = std::move(x.m_sample);
}

eprosima::micrortps::DATA_Payload_Sample &eprosima::micrortps::DATA_Payload_Sample::
operator=(const DATA_Payload_Sample &x)
{
    BaseObjectReply::operator=(x);
    m_sample                 = x.m_sample;

    return *this;
}

eprosima::micrortps::DATA_Payload_Sample &eprosima::micrortps::DATA_Payload_Sample::operator=(DATA_Payload_Sample &&x)
{
    BaseObjectReply::operator=(x);
    m_sample                 = std::move(x.m_sample);

    return *this;
}

size_t eprosima::micrortps::DATA_Payload_Sample::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += eprosima::micrortps::BaseObjectReply::getMaxCdrSerializedSize(current_alignment);
    current_alignment += eprosima::micrortps::Sample::getMaxCdrSerializedSize(current_alignment);

    return current_alignment - initial_alignment;
}

size_t
eprosima::micrortps::DATA_Payload_Sample::getCdrSerializedSize(const eprosima::micrortps::DATA_Payload_Sample &data,
                                                               size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += eprosima::micrortps::BaseObjectReply::getCdrSerializedSize(data, current_alignment);
    current_alignment += eprosima::micrortps::Sample::getCdrSerializedSize(data.sample(), current_alignment);

    return current_alignment - initial_alignment;
}

void eprosima::micrortps::DATA_Payload_Sample::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    BaseObjectReply::serialize(scdr);
    scdr << m_sample;
}

void eprosima::micrortps::DATA_Payload_Sample::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    BaseObjectReply::deserialize(scdr);
    dcdr >> m_sample;
}

eprosima::micrortps::DATA_Payload_DataSeq::DATA_Payload_DataSeq()
{
}

eprosima::micrortps::DATA_Payload_DataSeq::~DATA_Payload_DataSeq()
{
}

eprosima::micrortps::DATA_Payload_DataSeq::DATA_Payload_DataSeq(const DATA_Payload_DataSeq &x) : BaseObjectReply(x)
{
    m_data_seq = x.m_data_seq;
}

eprosima::micrortps::DATA_Payload_DataSeq::DATA_Payload_DataSeq(DATA_Payload_DataSeq &&x) : BaseObjectReply(x)
{
    m_data_seq = std::move(x.m_data_seq);
}

eprosima::micrortps::DATA_Payload_DataSeq &eprosima::micrortps::DATA_Payload_DataSeq::
operator=(const DATA_Payload_DataSeq &x)
{
    BaseObjectReply::operator=(x);
    m_data_seq               = x.m_data_seq;

    return *this;
}

eprosima::micrortps::DATA_Payload_DataSeq &eprosima::micrortps::DATA_Payload_DataSeq::
operator=(DATA_Payload_DataSeq &&x)
{
    BaseObjectReply::operator=(x);
    m_data_seq               = std::move(x.m_data_seq);

    return *this;
}

size_t eprosima::micrortps::DATA_Payload_DataSeq::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += eprosima::micrortps::BaseObjectReply::getMaxCdrSerializedSize(current_alignment);
    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
    for(size_t a = 0; a < 100; ++a)
    {
        current_alignment += eprosima::micrortps::SampleData::getMaxCdrSerializedSize(current_alignment);
    }

    return current_alignment - initial_alignment;
}

size_t
eprosima::micrortps::DATA_Payload_DataSeq::getCdrSerializedSize(const eprosima::micrortps::DATA_Payload_DataSeq &data,
                                                                size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += eprosima::micrortps::BaseObjectReply::getCdrSerializedSize(data, current_alignment);
    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
    for(size_t a = 0; a < data.data_seq().size(); ++a)
    {
        current_alignment +=
            eprosima::micrortps::SampleData::getCdrSerializedSize(data.data_seq().at(a), current_alignment);
    }

    return current_alignment - initial_alignment;
}

void eprosima::micrortps::DATA_Payload_DataSeq::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    BaseObjectReply::serialize(scdr);
    scdr << m_data_seq;
}

void eprosima::micrortps::DATA_Payload_DataSeq::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    BaseObjectReply::deserialize(scdr);
    dcdr >> m_data_seq;
}

eprosima::micrortps::DATA_Payload_SampleSeq::DATA_Payload_SampleSeq()
{
}

eprosima::micrortps::DATA_Payload_SampleSeq::~DATA_Payload_SampleSeq()
{
}

eprosima::micrortps::DATA_Payload_SampleSeq::DATA_Payload_SampleSeq(const DATA_Payload_SampleSeq &x)
    : BaseObjectReply(x)
{
    m_sample_seq = x.m_sample_seq;
}

eprosima::micrortps::DATA_Payload_SampleSeq::DATA_Payload_SampleSeq(DATA_Payload_SampleSeq &&x) : BaseObjectReply(x)
{
    m_sample_seq = std::move(x.m_sample_seq);
}

eprosima::micrortps::DATA_Payload_SampleSeq &eprosima::micrortps::DATA_Payload_SampleSeq::
operator=(const DATA_Payload_SampleSeq &x)
{
    BaseObjectReply::operator=(x);
    m_sample_seq             = x.m_sample_seq;

    return *this;
}

eprosima::micrortps::DATA_Payload_SampleSeq &eprosima::micrortps::DATA_Payload_SampleSeq::
operator=(DATA_Payload_SampleSeq &&x)
{
    BaseObjectReply::operator=(x);
    m_sample_seq             = std::move(x.m_sample_seq);

    return *this;
}

size_t eprosima::micrortps::DATA_Payload_SampleSeq::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += eprosima::micrortps::BaseObjectReply::getMaxCdrSerializedSize(current_alignment);
    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
    for(size_t a = 0; a < 100; ++a)
    {
        current_alignment += eprosima::micrortps::Sample::getMaxCdrSerializedSize(current_alignment);
    }

    return current_alignment - initial_alignment;
}

size_t eprosima::micrortps::DATA_Payload_SampleSeq::getCdrSerializedSize(
    const eprosima::micrortps::DATA_Payload_SampleSeq &data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += eprosima::micrortps::BaseObjectReply::getCdrSerializedSize(data, current_alignment);
    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
    for(size_t a = 0; a < data.sample_seq().size(); ++a)
    {
        current_alignment +=
            eprosima::micrortps::Sample::getCdrSerializedSize(data.sample_seq().at(a), current_alignment);
    }

    return current_alignment - initial_alignment;
}

void eprosima::micrortps::DATA_Payload_SampleSeq::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    BaseObjectReply::serialize(scdr);
    scdr << m_sample_seq;
}

void eprosima::micrortps::DATA_Payload_SampleSeq::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    BaseObjectReply::deserialize(scdr);
    dcdr >> m_sample_seq;
}

eprosima::micrortps::DATA_Payload_PackedSamples::DATA_Payload_PackedSamples()
{
}

eprosima::micrortps::DATA_Payload_PackedSamples::~DATA_Payload_PackedSamples()
{
}

eprosima::micrortps::DATA_Payload_PackedSamples::DATA_Payload_PackedSamples(const DATA_Payload_PackedSamples &x)
    : BaseObjectReply(x)
{
    m_packed_samples = x.m_packed_samples;
}

eprosima::micrortps::DATA_Payload_PackedSamples::DATA_Payload_PackedSamples(DATA_Payload_PackedSamples &&x)
    : BaseObjectReply(x)
{
    m_packed_samples = std::move(x.m_packed_samples);
}

eprosima::micrortps::DATA_Payload_PackedSamples &eprosima::micrortps::DATA_Payload_PackedSamples::
operator=(const DATA_Payload_PackedSamples &x)
{
    BaseObjectReply::operator=(x);
    m_packed_samples         = x.m_packed_samples;

    return *this;
}

eprosima::micrortps::DATA_Payload_PackedSamples &eprosima::micrortps::DATA_Payload_PackedSamples::
operator=(DATA_Payload_PackedSamples &&x)
{
    BaseObjectReply::operator=(x);
    m_packed_samples         = std::move(x.m_packed_samples);

    return *this;
}

size_t eprosima::micrortps::DATA_Payload_PackedSamples::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += eprosima::micrortps::BaseObjectReply::getMaxCdrSerializedSize(current_alignment);
    current_alignment += eprosima::micrortps::PackedSamples::getMaxCdrSerializedSize(current_alignment);

    return current_alignment - initial_alignment;
}

size_t eprosima::micrortps::DATA_Payload_PackedSamples::getCdrSerializedSize(
    const eprosima::micrortps::DATA_Payload_PackedSamples &data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += eprosima::micrortps::BaseObjectReply::getCdrSerializedSize(data, current_alignment);
    current_alignment +=
        eprosima::micrortps::PackedSamples::getCdrSerializedSize(data.packed_samples(), current_alignment);

    return current_alignment - initial_alignment;
}

void eprosima::micrortps::DATA_Payload_PackedSamples::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    BaseObjectReply::serialize(scdr);
    scdr << m_packed_samples;
}

void eprosima::micrortps::DATA_Payload_PackedSamples::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    BaseObjectReply::deserialize(dcdr);
    dcdr >> m_packed_samples;
}

eprosima::micrortps::WRITE_DATA_Payload::WRITE_DATA_Payload()
{
}

eprosima::micrortps::WRITE_DATA_Payload::~WRITE_DATA_Payload()
{
}

eprosima::micrortps::WRITE_DATA_Payload::WRITE_DATA_Payload(const WRITE_DATA_Payload &x) : BaseObjectRequest(x)
{
    m_data_to_write = x.m_data_to_write;
}

eprosima::micrortps::WRITE_DATA_Payload::WRITE_DATA_Payload(WRITE_DATA_Payload &&x) : BaseObjectRequest(x)
{
    m_data_to_write = std::move(x.m_data_to_write);
}

eprosima::micrortps::WRITE_DATA_Payload &eprosima::micrortps::WRITE_DATA_Payload::operator=(const WRITE_DATA_Payload &x)
{
    BaseObjectRequest::operator=(x);
    m_data_to_write            = x.m_data_to_write;

    return *this;
}

eprosima::micrortps::WRITE_DATA_Payload &eprosima::micrortps::WRITE_DATA_Payload::operator=(WRITE_DATA_Payload &&x)
{
    BaseObjectRequest::operator=(x);
    m_data_to_write            = std::move(x.m_data_to_write);

    return *this;
}

size_t eprosima::micrortps::WRITE_DATA_Payload::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += eprosima::micrortps::BaseObjectRequest::getMaxCdrSerializedSize(current_alignment);
    current_alignment += eprosima::micrortps::DataRepresentation::getMaxCdrSerializedSize(current_alignment);

    return current_alignment - initial_alignment;
}

size_t
eprosima::micrortps::WRITE_DATA_Payload::getCdrSerializedSize(const eprosima::micrortps::WRITE_DATA_Payload &data,
                                                              size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += eprosima::micrortps::BaseObjectRequest::getCdrSerializedSize(data, current_alignment);
    current_alignment +=
        eprosima::micrortps::DataRepresentation::getCdrSerializedSize(data.data_to_write(), current_alignment);

    return current_alignment - initial_alignment;
}

void eprosima::micrortps::WRITE_DATA_Payload::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    BaseObjectRequest::serialize(scdr);
    scdr << m_data_to_write;
}

void eprosima::micrortps::WRITE_DATA_Payload::deserialize(eprosima::fastcdr::Cdr &dcdr)
    BaseObjectRequest::deserialize(dcdr);
dcdr >> m_data_to_write;
}
